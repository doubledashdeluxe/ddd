#pragma once

#include <common/Array.hh>

class ExtendedSystemRecord {
public:
    Array<Array<char, 4>, 3> m_names;

    static ExtendedSystemRecord &Instance();

private:
    static ExtendedSystemRecord s_instance;
};
