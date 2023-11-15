#include "ResMgr.hh"

#include "game/KartLocale.hh"
#include "game/RaceInfo.hh"
#include "game/SysDebug.hh"

#include <common/Arena.hh>
#include <common/storage/Storage.hh>
#include <jsystem/JKRExpHeap.hh>
#include <jsystem/JKRSolidHeap.hh>
#include <payload/DOL.hh>

extern "C" {
#include <stdio.h>
}

void ResMgr::Create(JKRHeap *parentHeap) {
    for (u32 i = 0; i < 9; i++) {
        s_loaders[i] = nullptr;
    }
    for (u32 i = 0; i < 0x40; i++) {
        s_aramResArgs[i].status = 2;
    }
    s_loaders[ArchiveId::System] = JKRArchive::Mount(DOL::BinarySectionStart(), parentHeap,
            JKRArchive::MountDirection::Head);

    size_t keepHeapSize = 0xd80000;
    void *keepHeap = MEM2Arena::Instance()->alloc(keepHeapSize, 0x4);
    JKRHeap *keepHeapParentHeap = JKRExpHeap::Create(keepHeap, keepHeapSize, parentHeap, false);
    s_keepHeap = JKRSolidHeap::Create(UINT32_MAX, keepHeapParentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_keepHeap, "MRAM.arc");

    s_courseHeap = JKRSolidHeap::Create(0x500000, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
}

void ResMgr::LoadCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveId::Course;

    const char *base = GetCrsArcName(s_courseID);
    const char *languageName = KartLocale::LanguageName();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/CourseName/%s/%s_name.bti", languageName, base);
    s_courseName = LoadFile(path.values(), s_courseHeap);

    snprintf(path.values(), path.count(), "dvd:/StaffGhosts/%s.ght", base);
    s_staffGhost = LoadFile(path.values(), s_courseHeap);

    const char *suffix = s_courseOrder == 1 ? "L" : "";
    if (s_courseID == 0x24 && RaceInfo::Instance().getRaceLevel() != 0) {
        snprintf(path.values(), path.count(), "Course/Luigi2%s.arc", suffix);
    } else {
        snprintf(path.values(), path.count(), "Course/%s%s.arc", base, suffix);
    }
    s_loaders[ArchiveId::Course] = JKRArchive::Mount(path.values(), JKRArchive::MountMode::Mem,
            s_courseHeap, JKRArchive::MountDirection::Head);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveId::Course);
    s_loadFlag |= 1 << ArchiveId::Course;
}

void *ResMgr::LoadFile(const char *path, JKRHeap *heap) {
    Storage::FileHandle file(path, Storage::Mode::Read);
    u64 fileSize;
    if (!file.size(fileSize)) {
        return nullptr;
    }
    if (fileSize > UINT32_MAX) {
        return nullptr;
    }

    void *data = heap->alloc(fileSize, 0x20);
    if (!data) {
        return nullptr;
    }

    if (!file.read(data, fileSize, 0x0)) {
        heap->free(data);
        return nullptr;
    }

    return data;
}
