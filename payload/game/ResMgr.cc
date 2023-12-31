#include "ResMgr.hh"

#include "game/KartLocale.hh"
#include "game/RaceInfo.hh"
#include "game/SysDebug.hh"
#include "game/System.hh"

#include <common/Arena.hh>
#include <common/Array.hh>
#include <jsystem/JKRExpHeap.hh>
#include <jsystem/JKRSolidHeap.hh>
#include <payload/DOL.hh>
#include <payload/FileLoader.hh>

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
            JKRArchive::MountDirection::Head, true);

    size_t keepHeapSize = 0xd80000;
    void *keepHeap = MEM2Arena::Instance()->alloc(keepHeapSize, 0x4);
    s_keepHeap = JKRExpHeap::Create(keepHeap, keepHeapSize, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_keepHeap, "MRAM.arc");

    s_courseHeap = JKRSolidHeap::Create(0x500000, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
}

void ResMgr::LoadCourseData(u32 courseID, u32 courseOrder) {
    s_musicID = courseID;
    REPLACED(LoadCourseData)(courseID, courseOrder);
}

void ResMgr::LoadExtendedCourseData(const CourseManager::Course *course, u32 courseOrder) {
    s_musicID = course->musicID();
    s_courseID = course->courseID();
    s_courseOrder = courseOrder;
    s_courseHeap->freeAll();
    s_loadFlag &= ~(1 << ArchiveId::Course);
    s_loaders[ArchiveId::Course] = nullptr;
    void *userData = const_cast<CourseManager::Course *>(course);
    System::GetLoadTask()->request(LoadExtendedCourseData, userData, nullptr);
}

void ResMgr::LoadCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveId::Course;

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
    s_loaders[ArchiveId::Course] =
            JKRArchive::Mount(path.values(), s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveId::Course);
    s_loadFlag |= 1 << ArchiveId::Course;
}

void ResMgr::LoadExtendedCourseData(void *userData) {
    s_loadingFlag |= 1 << ArchiveId::Course;

    const CourseManager::Course *course = reinterpret_cast<CourseManager::Course *>(userData);
    s_courseName = course->loadLogo(s_courseHeap);
    s_staffGhost = course->loadStaffGhost(s_courseHeap);
    u32 raceLevel = RaceInfo::Instance().getRaceLevel();
    void *courseArchive = course->loadCourse(s_courseOrder, raceLevel, s_courseHeap);
    s_loaders[ArchiveId::Course] =
            JKRArchive::Mount(courseArchive, s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveId::Course);
    s_loadFlag |= 1 << ArchiveId::Course;
}

u32 ResMgr::GetMusicID() {
    return s_musicID;
}

u32 ResMgr::GetCourseID() {
    return s_courseID;
}

u32 ResMgr::s_musicID;
