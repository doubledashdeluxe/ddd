#pragma once

#include "payload/Mutex.hh"

#include <portable/crypto/Random.hh>

class CubeRandom : public Random {
public:
    void get(void *data, size_t size) override;
    u32 get(u32 range) override;

    static void Init();
    static CubeRandom *Instance();

private:
    CubeRandom();

    bool initWithDiscTimings();
    void initWithES();

    Mutex m_mutex;

    static CubeRandom *s_instance;
};
