#pragma once

#include "jsystem/JKRArchive.hh"

#include <payload/Archive.hh>

class JKRMemArchive : public JKRArchive {
public:
    JKRMemArchive(s32 entrynum, Archive archive, u32 mountDirection, bool ownsMemory);
    ~JKRMemArchive() override;
    void vf_24() override;
    void vf_28() override;
    void vf_3c() override;
    void vf_40() override;
    void vf_44() override;

private:
    u8 *m_archive;
    u8 *m_files;
    bool m_ownsMemory;
};
size_assert(JKRMemArchive, 0x70);
