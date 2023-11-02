#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class JUTConsole {
public:
    class OutputFlag {
    public:
        enum {
            None = 0 << 0,
            OSReport = 1 << 0,
            Console = 1 << 1,
            All = (1 << 2) - 1,
        };

    private:
        OutputFlag();
    };

    void REPLACED(print)(const char *message);
    REPLACE void print(const char *message);

private:
    u8 _00[0x58 - 0x00];
    int m_outputFlag;
    u8 _5C[0x6C - 0x5C];
};
size_assert(JUTConsole, 0x6C);
