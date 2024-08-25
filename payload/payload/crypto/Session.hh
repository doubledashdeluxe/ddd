#pragma once

#include <common/Array.hh>

class Session {
public:
    Session();
    ~Session();

    Array<u8, 32> m_readK;
    Array<u8, 32> m_writeK;
};
