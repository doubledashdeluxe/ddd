#pragma once

#include <portable/SC.hh>

class SCFile : public SC {
public:
    SCFile();
    ~SCFile();

private:
    alignas(0x20) Array<u8, 0x4000> m_buffer;
};
