#pragma once

#include <jsystem/JKRArchive.hh>
#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class ResMgr {
public:
    class ArchiveId {
    public:
        enum {
            Course = 0x3,
            System = 0x4,
        };

    private:
        ArchiveId();
    };

    REPLACE static void Create(JKRHeap *parentHeap);
    static void LoadKeepData();
    static void LoadCourseData(u32 courseId, u32 courseOrder);

private:
    struct AramResArg {
        u8 _00[0x10 - 0x00];
        u32 status;
        u8 _14[0x18 - 0x14];
    };
    size_assert(AramResArg, 0x18);

    ResMgr();

    static const char *GetCrsArcName(u32 courseID);
    REPLACE static void LoadCourseData(void *userData);
    static void *LoadFile(const char *path, JKRHeap *heap);

    static JKRArchive *s_loaders[9];
    static AramResArg s_aramResArgs[0x40];
    static JKRHeap *s_keepHeap;
    static JKRHeap *s_courseHeap;
    static u32 s_loadFlag;
    static u32 s_loadingFlag;
    static u32 s_courseID;
    static u32 s_courseOrder;
    static u32 s_mountCourseID;
    static u32 s_mountCourseOrder;
    static void *s_courseName;
    static void *s_staffGhost;
};
size_assert(ResMgr, 0x1);
