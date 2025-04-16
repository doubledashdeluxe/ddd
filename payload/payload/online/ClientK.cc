#include "ClientK.hh"

#include "payload/crypto/Random.hh"

#include <common/Arena.hh>
#include <common/Bytes.hh>
#include <common/ECID.hh>
#include <common/FS.hh>
#include <common/Platform.hh>
#include <common/ios/File.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSArena.h>
#include <monocypher/monocypher.h>
}
#include <portable/Align.hh>

extern "C" {
#include <assert.h>
}

void ClientK::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x20) Array<u8, 32>;

    u32 lo = reinterpret_cast<uintptr_t>(OSGetArenaLo());
    u32 hi = reinterpret_cast<uintptr_t>(OSGetArenaHi());
    lo = AlignUp<u32>(lo, 8);
    hi = AlignDown<u32>(hi, 8);
    u32 blockCount = 12 * 1024;
    assert(lo + blockCount * 1024 <= hi);
    void *blocks = reinterpret_cast<void *>(lo);

    ECID ecid = ECID::Get();
    Array<u8, 0xc> pass;
    Bytes::WriteBE<u32>(pass.values(), 0, ecid.u);
    Bytes::WriteBE<u32>(pass.values(), 4, ecid.m);
    Bytes::WriteBE<u32>(pass.values(), 8, ecid.l);

    alignas(0x20) Array<u8, 32> key;
    if (Platform::IsGameCube()) {
        const char *path = "main:/ddd/secret.bin";
        u32 keySize;
        if (!Storage::ReadFile(path, key.values(), key.count(), &keySize) ||
                keySize != key.count()) {
            Random::Get(key.values(), key.count());
            Storage::WriteFile(path, key.values(), key.count(), Storage::Mode::WriteAlways);
        }
    } else {
        const char *path = "/title/00010008/44444443/data/secret.bin";
        IOS::File file(path, IOS::Mode::Read);
        if (!file.ok() || file.read(key.values(), key.count()) != static_cast<s32>(key.count())) {
            Random::Get(key.values(), key.count());
            FS().writeFile(path, key.values(), key.count(), 0660);
        }
    }

    crypto_argon2_config config;
    config.algorithm = CRYPTO_ARGON2_ID;
    config.nb_blocks = blockCount;
    config.nb_passes = 3;
    config.nb_lanes = 1;
    crypto_argon2_inputs inputs;
    inputs.pass = pass.values();
    inputs.pass_size = pass.count();
    inputs.salt = nullptr;
    inputs.salt_size = 0;
    crypto_argon2_extras extras;
    extras.key = key.values();
    extras.key_size = key.count();
    extras.ad = nullptr;
    extras.ad_size = 0;
    crypto_argon2(s_instance->values(), s_instance->count(), blocks, config, inputs, extras);
    crypto_wipe(pass.values(), pass.count());
    crypto_wipe(key.values(), key.count());
}

const Array<u8, 32> &ClientK::Get() {
    return *s_instance;
}

Array<u8, 32> *ClientK::s_instance = nullptr;
