#pragma once

#include <common/Array.hh>
#include <portable/Types.hh>

class SRAM {
public:
    class Language {
    public:
        enum {
            English = 0,
            German = 1,
            French = 2,
            Spanish = 3,
            Italian = 4,
            Dutch = 5,
        };

    private:
        Language();
    };

    SRAM();
    ~SRAM();
    bool ok() const;
    bool getLanguage(u8 &language) const;

private:
    bool m_ok;
    Array<u8, 64> m_buffer;
};
