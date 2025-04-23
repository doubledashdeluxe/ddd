#include "Random.hh"

#ifdef __CWCC__
#include "payload/Lock.hh"

#include <common/Arena.hh>
#include <common/Clock.hh>
#include <common/ES.hh>
#include <common/Platform.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <monocypher/monocypher.h>
}
#include <portable/Algorithm.hh>

extern "C" {
#include <assert.h>
#include <string.h>
}

void Random::Init() {
    assert(!s_isInit);

    s_mutex = new (MEM1Arena::Instance(), 0x4) Mutex;
    assert(s_mutex);

    if (Platform::IsGameCube()) {
        while (!InitWithDiscTimings()) {
            Clock::WaitSeconds(1);
        }
    } else {
        InitWithES();
    }

    s_isInit = true;
}

void Random::Get(void *data, size_t size) {
    assert(s_isInit);

    Lock<Mutex> lock(*s_mutex);

    while (size > 0) {
        if (s_offset == s_buffer.count()) {
            Array<u8, 8> nonce(0);
            crypto_chacha20_djb(s_buffer.values(), nullptr, s_buffer.count(), s_buffer.values(),
                    nonce.values(), 0);
            s_offset = 32;
        }

        size_t chunkSize = Min(size, s_buffer.count() - s_offset);
        memcpy(data, s_buffer.values() + s_offset, chunkSize);
        data = reinterpret_cast<u8 *>(data) + chunkSize;
        size -= chunkSize;
        s_offset += chunkSize;
    }

    crypto_wipe(s_buffer.values() + 32, s_offset - 32);
}

u32 Random::Get(u32 range) {
    range--;
    u32 zero = 0;
    u32 mask = ~zero;
#ifdef __CWCC__
    mask >>= __cntlzw(range | 1);
#else
    mask >>= __builtin_clz(range | 1);
#endif
    uint32_t x;
    do {
        Get(&x, sizeof(x));
        x &= mask;
    } while (x > range);
    return x;
}

bool Random::InitWithDiscTimings() {
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
            s_buffer[i] &= ~(1 << j);
            s_buffer[i] |= ((now - start) & 1) << j;
            start = now;
        }
    }
    return true;
}

void Random::InitWithES() {
    ES es;
    assert(es.ok());

    Array<u8, 0x3c> signature;
    Array<u8, 0x180> certificate;
    assert(es.sign(nullptr, 0, signature, certificate));
    memcpy(s_buffer.values(), signature.values(), 32);
}

bool Random::s_isInit = false;
Mutex *Random::s_mutex = nullptr;
Array<u8, 32 + 256> Random::s_buffer;
u16 Random::s_offset = s_buffer.count();
#endif
