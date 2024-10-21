#pragma once

#include "jsystem/JKRAramBlock.hh"
#include "jsystem/JKRArchive.hh"

#include <common/UniquePtr.hh>

class JKRAramArchive : public JKRArchive {
public:
    JKRAramArchive(s32 entrynum, Archive archive, u32 mountDirection);
    ~JKRAramArchive() override;
    void vf_3c() override;
    void vf_40() override;
    void vf_44() override;

private:
    UniquePtr<JKRAramBlock> m_block;
    UniquePtr<u8> m_archive; // Modified
};
size_assert(JKRAramArchive, 0x6c);
