#include "ResMgr.hh"

#include "game/CourseID.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartLocale.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/SysDebug.hh"
#include "game/System.hh"

#include <common/Arena.hh>
#include <common/Array.hh>
#include <jsystem/JKRExpHeap.hh>
#include <jsystem/JKRSolidHeap.hh>
#include <payload/Archive.hh>
#include <payload/DOLBinary.hh>
#include <payload/FileLoader.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
}

void ResMgr::Create(JKRHeap *parentHeap) {
    for (u32 i = 0; i < 9; i++) {
        s_loaders[i] = nullptr;
    }
    for (u32 i = 0; i < 0x40; i++) {
        s_aramResArgs[i].status = 2;
    }
    s_loaders[ArchiveID::System] = JKRArchive::Mount(DOLBinary::BinarySectionStart(), parentHeap,
            JKRArchive::MountDirection::Head);

    size_t keepHeapSize = 0x7c0000;
    void *keepHeapPtr = MEM2Arena::Instance()->alloc(keepHeapSize, 0x4);
    s_keepHeap = JKRExpHeap::Create(keepHeapPtr, keepHeapSize, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_keepHeap, "MRAM.arc");

    s_courseHeap = JKRSolidHeap::Create(0x280000, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
}

const JKRHeap *ResMgr::GetCourseHeap() {
    return s_courseHeap;
}

void ResMgr::LoadKeepData(void * /* userData */) {
    if (!(s_loadFlag & 1 << ArchiveID::ARAM)) {
        s_loadingFlag |= 1 << ArchiveID::ARAM;

        s_loaders[ArchiveID::ARAM] = JKRArchive::Mount("/ARAM.arc", JKRArchive::MountMode::Aram,
                s_keepHeap, JKRArchive::MountDirection::Head);

        s_loadingFlag &= ~(1 << ArchiveID::ARAM);
        s_loadFlag |= 1 << ArchiveID::ARAM;
    }

    if (!(s_loadFlag & 1 << ArchiveID::MRAM)) {
        s_loadingFlag |= 1 << ArchiveID::MRAM;

        s_loaders[ArchiveID::MRAM] = JKRArchive::Mount("/MRAM.arc", JKRArchive::MountMode::Mem,
                s_keepHeap, JKRArchive::MountDirection::Head);

        s_loadingFlag &= ~(1 << ArchiveID::MRAM);
        s_loadFlag |= 1 << ArchiveID::MRAM;
    }

    if (!(s_loadFlag & 1 << ArchiveID::Race2D)) {
        s_loadingFlag |= 1 << ArchiveID::Race2D;

        Storage::FileHandle file("dvd:/MRAM.arc", Storage::Mode::Read);
        u64 mramArchiveSize;
        assert(file.size(mramArchiveSize) && mramArchiveSize <= UINT32_MAX);

        alignas(0x20) Array<u8, 0x20> mramArchiveHeader;
        assert(file.read(mramArchiveHeader.values(), mramArchiveHeader.count(), 0x00));
        Archive mramArchive(mramArchiveHeader.values());
        assert(mramArchive.isHeaderValid(mramArchiveSize));

        Archive::Tree mramTree(new (s_keepHeap, 0x20) u8[mramArchive.getTreeSize()]);
        assert(file.read(mramTree.get(), mramArchive.getTreeSize(), mramArchive.getTreeOffset()));
        assert(mramTree.isValid(mramArchive.getTreeSize(), mramArchive.getFilesSize()));

        const char *name;
        Archive::Dir dir(nullptr);
        Archive::Node node(nullptr);
        bool exists;
        assert(mramTree.search("/Race2D.arc", name, dir, node, exists));
        assert(exists && node.isFile());
        u32 race2DArchiveOffset = mramArchive.getFilesOffset() + node.getFileOffset();
        u32 race2DArchiveSize = node.getFileSize();
        delete[] mramTree.get();

        u8 *race2DArchive = new (s_keepHeap, 0x20) u8[race2DArchiveSize];
        assert(file.read(race2DArchive, race2DArchiveSize, race2DArchiveOffset));
        s_loaders[ArchiveID::Race2D] =
                JKRArchive::Mount(race2DArchive, race2DArchiveSize, JKRArchive::MountMode::Mem,
                        s_keepHeap, JKRArchive::MountDirection::Tail, true, true);

        s_loadingFlag &= ~(1 << ArchiveID::Race2D);
        s_loadFlag |= 1 << ArchiveID::Race2D;
    }

    if (!(s_loadFlag & 1 << ArchiveID::MRAMLoc)) {
        s_loadingFlag |= 1 << ArchiveID::MRAMLoc;

        Array<char, 64> path;
        snprintf(path.values(), path.count(), "/MRAM_Locale/%s/MRAMLoc.arc",
                KartLocale::GetLanguageName());
        s_loaders[ArchiveID::MRAMLoc] = JKRArchive::Mount(path.values(), JKRArchive::MountMode::Mem,
                s_keepHeap, JKRArchive::MountDirection::Head);

        Kart2DCommon::Create(s_keepHeap);

        s_loadingFlag &= ~(1 << ArchiveID::MRAMLoc);
        s_loadFlag |= 1 << ArchiveID::MRAMLoc;
    }
}

void ResMgr::LoadCourseData(u32 courseID, u32 courseOrder) {
    delete s_loaders[ArchiveID::Course];
    s_course = nullptr;
    s_musicID = courseID;
    REPLACED(LoadCourseData)(courseID, courseOrder);
}

void ResMgr::LoadExtendedCourseData(const CourseManager::Course *course, u32 courseOrder) {
    delete s_loaders[ArchiveID::Course];
    s_course = course;
    s_musicID = course->musicID();
    s_courseOrder = courseOrder;
    s_courseHeap->freeAll();
    s_loadFlag &= ~(1 << ArchiveID::Course);
    s_loaders[ArchiveID::Course] = nullptr;
    System::GetLoadTask()->request(LoadExtendedCourseData, nullptr, nullptr);
}

void ResMgr::LoadCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveID::Course;

    const char *base = GetCrsArcName(s_courseID);
    const char *languageName = KartLocale::GetLanguageName();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/CourseName/%s/%s_name.bti", languageName, base);
    s_courseName = FileLoader::Load(path.values(), s_courseHeap);

    snprintf(path.values(), path.count(), "dvd:/StaffGhosts/%s.ght", base);
    s_staffGhost = FileLoader::Load(path.values(), s_courseHeap);

    const char *suffix = s_courseOrder == 1 ? "L" : "";
    if (s_courseID == 0x24 && RaceInfo::Instance().getRaceLevel() != 0) {
        snprintf(path.values(), path.count(), "Course/Luigi2%s.arc", suffix);
    } else {
        snprintf(path.values(), path.count(), "Course/%s%s.arc", base, suffix);
    }
    s_loaders[ArchiveID::Course] = JKRArchive::Mount(path.values(), JKRArchive::MountMode::Mem,
            s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveID::Course);
    s_loadFlag |= 1 << ArchiveID::Course;
}

void ResMgr::LoadExtendedCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveID::Course;

    s_courseName = s_course->loadLogo(s_courseHeap);
    s_staffGhost = s_course->loadStaffGhost(s_courseHeap);
    u32 raceLevel = RaceInfo::Instance().getRaceLevel();
    u32 courseArchiveSize;
    void *courseArchive =
            s_course->loadCourse(s_courseOrder, raceLevel, s_courseHeap, courseArchiveSize);
    s_loaders[ArchiveID::Course] =
            JKRArchive::Mount(courseArchive, courseArchiveSize, JKRArchive::MountMode::Mem,
                    s_courseHeap, JKRArchive::MountDirection::Head, false, false);

    for (s_courseID = CourseID::BabyLuigi; s_courseID <= CourseID::Mini8; s_courseID++) {
        const char *base = GetCrsArcName(s_courseID);
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "%s_course.bol", base);
        if (GetPtr(ArchiveID::Course, path.values())) {
            break;
        }
    }
    assert(s_courseID >= CourseID::BabyLuigi && s_courseID <= CourseID::Mini8);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveID::Course);
    s_loadFlag |= 1 << ArchiveID::Course;
}

void *ResMgr::GetPtr(u32 courseDataID) {
    if (courseDataID == CourseDataID::MapBti && s_course && s_course->minimapConfig()) {
        // Hack: this should be done in the Race2D constructor, but it's just way too long
        Race2D::Instance()->setMinimapConfig(*s_course->minimapConfig());
    }

    return REPLACED(GetPtr)(courseDataID);
}

void *ResMgr::GetPtr(u32 archiveID, const char *path) {
    if (archiveID == ArchiveID::MRAM && !strcmp(path, "Race2D.arc")) {
        return reinterpret_cast<void *>(s_loaders[ArchiveID::Race2D]->getEntrynum());
    }

    return REPLACED(GetPtr)(archiveID, path);
}

const CourseManager::Course *ResMgr::GetCourse() {
    return s_course;
}

u32 ResMgr::GetMusicID() {
    return s_musicID;
}

u32 ResMgr::GetCourseID() {
    return s_courseID;
}

const CourseManager::Course *ResMgr::s_course;
u32 ResMgr::s_musicID;
