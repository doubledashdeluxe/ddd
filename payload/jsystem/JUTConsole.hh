#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

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
    void print_f(const char *message, ...);

private:
    u8 _00[0x58 - 0x00];
    int m_outputFlag;
    u8 _5C[0x6C - 0x5C];
};
size_assert(JUTConsole, 0x6C);

class JUTConsoleManager {
public:
    void drawDirect(bool waitRetrace) const;

    static JUTConsoleManager *Instance();

private:
    static JUTConsoleManager *s_manager;
};
