#pragma once

#include <portable/Array.hh>

class ExtendedSystemRecord {
public:
    Array<Array<char, 4>, 15> m_names;

    static ExtendedSystemRecord &Instance();

private:
    static ExtendedSystemRecord s_instance;
};
