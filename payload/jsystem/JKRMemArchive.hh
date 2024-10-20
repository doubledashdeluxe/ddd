#pragma once

#include "jsystem/JKRArchive.hh"

#include <payload/Archive.hh>

class JKRMemArchive : public JKRArchive {
public:
    JKRMemArchive(s32 entrynum, Archive archive, u32 mountDirection, bool ownsMemory);
    ~JKRMemArchive();

private:
    void *m_archive;
    u8 *m_files;
    bool m_ownsMemory;
};
size_assert(JKRMemArchive, 0x70);
