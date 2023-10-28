#pragma once

#include "jsystem/JKRArchive.hh"

#include <payload/Replace.hh>

class JKRMemArchive : public JKRArchive {
public:
    class MemBreakFlag {
    public:
        enum {
            NoBreak = 0,
            Break = 1,
        };

    private:
        MemBreakFlag();
    };

    JKRMemArchive(s32 entrynum, u32 mountDirection);
    JKRMemArchive(void *archive, u32 r5, u32 memBreakFlag);
    ~JKRMemArchive();

    REPLACE bool open(s32 entrynum, u32 mountDirection);

private:
    void *m_archive;
    u8 *m_files;
    bool m_ownsMemory;
};
static_assert(sizeof(JKRMemArchive) == 0x70);
