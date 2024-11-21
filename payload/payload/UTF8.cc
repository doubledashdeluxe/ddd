#include "UTF8.hh"

extern "C" {
#include <utf8decode.h>

#include <ctype.h>
}

u8 UTF8::ToAscii(u32 c) {
    switch (c) {
    case 0x00c0: // À
    case 0x00c1: // Á
    case 0x00c2: // Â
    case 0x00c3: // Ã
    case 0x00c4: // Ä
    case 0x00c5: // Å
        return 'A';
    case 0x00c7: // Ç
        return 'C';
    case 0x00c8: // È
    case 0x00c9: // É
    case 0x00ca: // Ê
    case 0x00cb: // Ë
        return 'E';
    case 0x00cc: // Ì
    case 0x00cd: // Í
    case 0x00ce: // Î
    case 0x00cf: // Ï
        return 'I';
    case 0x00d1: // Ñ
        return 'N';
    case 0x00d2: // Ò
    case 0x00d3: // Ó
    case 0x00d4: // Ô
    case 0x00d5: // Õ
    case 0x00d6: // Ö
        return 'O';
    case 0x00d9: // Ù
    case 0x00da: // Ú
    case 0x00db: // Û
    case 0x00dc: // Ü
        return 'U';
    case 0x00dd: // Ý
        return 'Y';
    case 0x00e0: // à
    case 0x00e1: // á
    case 0x00e2: // â
    case 0x00e3: // ã
    case 0x00e4: // ä
    case 0x00e5: // å
        return 'a';
    case 0x00e7: // ç
        return 'c';
    case 0x00e8: // è
    case 0x00e9: // é
    case 0x00ea: // ê
    case 0x00eb: // ë
        return 'e';
    case 0x00ec: // ì
    case 0x00ed: // í
    case 0x00ee: // î
    case 0x00ef: // ï
        return 'i';
    case 0x00f1: // ñ
        return 'n';
    case 0x00f2: // ò
    case 0x00f3: // ó
    case 0x00f4: // ô
    case 0x00f5: // õ
    case 0x00f6: // ö
        return 'o';
    case 0x00f9: // ù
    case 0x00fa: // ú
    case 0x00fb: // û
    case 0x00fc: // ü
        return 'u';
    case 0x00fd: // ý
    case 0x00ff: // ÿ
        return 'y';
    default:
        return c < 0x0080 ? c : 0x80;
    }
}

u32 UTF8::ToLower(u32 c) {
    if (c < 0x0080) {
        return tolower(c);
    }
    if (c >= 0x00c0 && c <= 0x00de && c != 0x00d7) {
        return c + 0x20;
    }
    return c;
}

u32 UTF8::Length(const char *s) {
    u32 length = 0;
    while (Next(s)) {
        length++;
    }
    return length;
}

u32 UTF8::Next(const char *&s) {
    for (u32 prev = UTF8_ACCEPT, curr = UTF8_ACCEPT, c; *s; prev = curr) {
        switch (utf8decode(&curr, &c, static_cast<u8>(*s++))) {
        case UTF8_ACCEPT:
            return c;
        case UTF8_REJECT:
            if (prev != UTF8_ACCEPT) {
                s--;
            }
            return 0xfffd;
        }
    }
    return '\0';
}

s32 UTF8::Compare(const char *s1, const char *s2) {
    while (true) {
        u32 c1 = Next(s1), c2 = Next(s2);

        u8 c1a = ToAscii(c1), c2a = ToAscii(c2);
        if (c1a != c2a) {
            return c1a < c2a ? -1 : 1;
        }

        if (c1 != c2) {
            return c1 < c2 ? -1 : 1;
        }

        if (c1 == '\0') {
            return 0;
        }
    }
}

s32 UTF8::CaseCompare(const char *s1, const char *s2) {
    while (true) {
        u32 c1 = ToLower(Next(s1)), c2 = ToLower(Next(s2));

        u8 c1a = ToAscii(c1), c2a = ToAscii(c2);
        if (c1a != c2a) {
            return c1a < c2a ? -1 : 1;
        }

        if (c1 != c2) {
            return c1 < c2 ? -1 : 1;
        }

        if (c1 == '\0') {
            return 0;
        }
    }
}
