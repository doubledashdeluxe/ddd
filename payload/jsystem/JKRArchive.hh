#pragma once

#include "jsystem/JKRFileLoader.hh"
#include "jsystem/JKRHeap.hh"

class JKRArchive : public JKRFileLoader {
public:
    class MountMode {
    public:
        enum {
            None = 0,
            Mem = 1,
            Aram = 2,
            Dvd = 3,
            Comp = 4,
        };

    private:
        MountMode();
    };

    class MountDirection {
    public:
        enum {
            Head = 1,
            Tail = 2,
        };

    private:
        MountDirection();
    };

    JKRArchive(s32 entrynum, u32 mountMode);
    ~JKRArchive();
    // ...

protected:
    JKRHeap *m_heap;
    u8 m_mountMode;
    u8 _3d[0x44 - 0x3d];
    u8 *m_tree;
    u8 *m_dirs;
    u8 *m_nodes;
    u8 _50[0x54 - 0x50];
    char *m_names;
    u8 _58[0x60 - 0x58];
    u32 m_mountDirection;
};
size_assert(JKRArchive, 0x64);
