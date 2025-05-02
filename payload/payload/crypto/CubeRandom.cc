#include "CubeRandom.hh"

#include "payload/Lock.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <cube/ES.hh>
#include <cube/Platform.hh>
#include <cube/storage/Storage.hh>

extern "C" {
#include <assert.h>
#include <string.h>
}

void CubeRandom::get(void *data, size_t size) {
    Lock<Mutex> lock(m_mutex);

    Random::get(data, size);
}

u32 CubeRandom::get(u32 range) {
    Lock<Mutex> lock(m_mutex);

    return Random::get(range);
}

void CubeRandom::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) CubeRandom;
    assert(s_instance);
}

CubeRandom *CubeRandom::Instance() {
    return s_instance;
}

CubeRandom::CubeRandom() {
    if (Platform::IsGameCube()) {
        while (!initWithDiscTimings()) {
            Clock::WaitSeconds(1);
        }
    } else {
        initWithES();
    }
}

bool CubeRandom::initWithDiscTimings() {
    Storage::FileHandle file("dvd:/Movie/play1.thp", Storage::Mode::Read);
    alignas(0x20) Array<u8, 256> buffer;
    if (!file.read(buffer.values(), buffer.count(), 0)) {
        return false;
    }
    s64 start = Clock::GetMonotonicTicks();
    for (u32 i = 0; i < 32; i++) {
        for (u32 j = 0; j < 8; j++) {
            if (!file.read(buffer.values(), buffer.count(), (1 + i) * 4096)) {
                return false;
            }
            s64 now = Clock::GetMonotonicTicks();
            m_buffer[i] &= ~(1 << j);
            m_buffer[i] |= ((now - start) & 1) << j;
            start = now;
        }
    }
    return true;
}

void CubeRandom::initWithES() {
    ES es;
    assert(es.ok());

    Array<u8, 0x3c> signature;
    Array<u8, 0x180> certificate;
    assert(es.sign(nullptr, 0, signature, certificate));
    memcpy(m_buffer.values(), signature.values(), 32);
}

CubeRandom *CubeRandom::s_instance = nullptr;
