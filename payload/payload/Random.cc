#include "Random.hh"

#include "payload/Lock.hh"

#include <common/Algorithm.hh>
#include <common/Arena.hh>
#include <common/ES.hh>
extern "C" {
#include <monocypher/monocypher.h>

#include <assert.h>
#include <string.h>
}

void Random::Init() {
    assert(!s_isInit);

    s_mutex = new (MEM1Arena::Instance(), 0x4) Mutex;
    assert(s_mutex);

    ES es;
    assert(es.ok());

    Array<u8, 0x3c> signature;
    Array<u8, 0x180> certificate;
    assert(es.sign(nullptr, 0, signature, certificate));
    memcpy(s_buffer.values(), signature.values(), 32);

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

bool Random::s_isInit = false;
Mutex *Random::s_mutex = nullptr;
Array<u8, 32 + 256> Random::s_buffer;
u16 Random::s_offset = s_buffer.count();
