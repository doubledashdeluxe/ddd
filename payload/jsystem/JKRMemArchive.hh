#pragma once

#include "jsystem/JKRArchive.hh"

#include <common/Array.hh>
#include <payload/Replace.hh>

class JKRMemArchive : public JKRArchive {
public:
    class MemBreakFlag {
    public:
        enum {
            DoNotBreak = 0,
            Break = 1,
        };

    private:
        MemBreakFlag();
    };

    JKRMemArchive(s32 entrynum, u32 mountDirection, bool patchesAllowed);
    JKRMemArchive(void *archive, u32 mountDirection, bool patchesAllowed);
    ~JKRMemArchive();

private:
    bool open(s32 entrynum, u32 mountDirection, bool patchesAllowed);
    bool open(void *archive, u32 mountDirection, bool patchesAllowed);

    bool parseTree(u32 treeSize);
    bool addSubnodes(u64 &archiveSize, const char *prefix, const char *bare);
    bool addSubdir(u64 &archiveSize, const char *prefix, const char *bare,
            Array<char, 256> &relative);
    bool addSubfile(u64 &archiveSize, const char *prefix, const char *bare,
            const Array<char, 256> &relative);
    bool searchNode(const Array<char, 256> &relative, const char *&name, u8 *&dir, u8 *&node,
            bool &exists);
    bool addNode(u64 &archiveSize, const char *name, u8 *&dir, u8 *&node);
    bool relocate(u64 &archiveSize, u8 *&dir, u8 *&node);
    bool allocAt(u64 &archiveSize, u64 newArchiveSize, void *ptr, u8 *&dir, u8 *&node);
    void move(u64 archiveSize, void *ptr, s32 offset, u8 *&dir, u8 *&node);

    void *m_archive;
    u8 *m_files;
    bool m_ownsMemory;
};
size_assert(JKRMemArchive, 0x70);
