#include "ResMgr.hh"

#include "game/CourseID.hh"
#include "game/KartLocale.hh"
#include "game/Race2D.hh"
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
    s_loaders[ArchiveID::System] = JKRArchive::Mount(DOL::BinarySectionStart(), parentHeap,
            JKRArchive::MountDirection::Head, true);

    size_t keepHeapSize = 0xd80000;
    void *keepHeap = MEM2Arena::Instance()->alloc(keepHeapSize, 0x4);
    s_keepHeap = JKRExpHeap::Create(keepHeap, keepHeapSize, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_keepHeap, "MRAM.arc");

    s_courseHeap = JKRSolidHeap::Create(0x280000, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
}

void ResMgr::LoadCourseData(u32 courseID, u32 courseOrder) {
    delete s_loaders[ArchiveID::Course];
    s_musicID = courseID;
    REPLACED(LoadCourseData)(courseID, courseOrder);
}

void ResMgr::LoadExtendedCourseData(const CourseManager::Course *course, u32 courseOrder) {
    delete s_loaders[ArchiveID::Course];
    s_musicID = course->musicID();
    s_courseID = course->courseID();
    s_courseOrder = courseOrder;
    CourseManager::Instance()->freeAll();
    s_loadFlag &= ~(1 << ArchiveID::Course);
    s_loaders[ArchiveID::Course] = nullptr;
    void *userData = const_cast<CourseManager::Course *>(course);
    System::GetLoadTask()->request(LoadExtendedCourseData, userData, nullptr);
}

void ResMgr::LoadCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveID::Course;

    s_minimapConfig = nullptr;

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
    s_loaders[ArchiveID::Course] =
            JKRArchive::Mount(path.values(), s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveID::Course);
    s_loadFlag |= 1 << ArchiveID::Course;
}

void ResMgr::LoadExtendedCourseData(void *userData) {
    s_loadingFlag |= 1 << ArchiveID::Course;

    const CourseManager::Course *course = reinterpret_cast<CourseManager::Course *>(userData);
    s_minimapConfig = course->minimapConfig();
    s_courseName = course->loadLogo();
    s_staffGhost = course->loadStaffGhost();
    u32 raceLevel = RaceInfo::Instance().getRaceLevel();
    void *courseArchive = course->loadCourse(s_courseOrder, raceLevel);
    s_loaders[ArchiveID::Course] =
            JKRArchive::Mount(courseArchive, s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveID::Course);
    s_loadFlag |= 1 << ArchiveID::Course;
}

void *ResMgr::GetPtr(u32 courseDataID) {
    if (courseDataID == CourseDataID::MapBti && s_minimapConfig) {
        // Hack: this should be done in the Race2D constructor, but it's just way too long
        Race2D::Instance()->setMinimapConfig(*s_minimapConfig);
    }

    const char *name;
    switch (courseDataID) {
    case CourseDataID::CourseBmd:
        name = "course.bmd";
        break;
    case CourseDataID::CourseBco:
        name = "course.bco";
        break;
    case CourseDataID::CourseBol:
        name = "course.bol";
        break;
    case CourseDataID::CourseBtk:
        name = "course.btk";
        break;
    case CourseDataID::CourseBtk2:
        name = "course_02.btk";
        break;
    case CourseDataID::CourseBtk3:
        name = "course_03.btk";
        break;
    case CourseDataID::CourseBtp:
        name = "course.btp";
        break;
    case CourseDataID::CourseBrk:
        name = "course.brk";
        break;
    case CourseDataID::SkyBmd:
        name = "sky.bmd";
        break;
    case CourseDataID::SkyBtk:
        name = "sky.btk";
        break;
    case CourseDataID::SkyBrk:
        name = "sky.brk";
        break;
    case CourseDataID::MapBti:
        name = "map.bti";
        break;
    case CourseDataID::CourseName:
        return s_courseName;
    case CourseDataID::StaffGhost:
        return s_staffGhost;
    default:
        return nullptr;
    }
    const char *base = GetCrsArcName(s_courseID);
    Array<char, 32> path;
    snprintf(path.values(), path.count(), "%s_%s", base, name);
    void *ptr = GetPtr(ArchiveID::Course, path.values());
    if (ptr) {
        return ptr;
    }
    for (u32 courseID = CourseID::BabyLuigi; courseID <= CourseID::Mini8; courseID++) {
        base = GetCrsArcName(courseID);
        snprintf(path.values(), path.count(), "%s_%s", base, name);
        void *ptr = GetPtr(ArchiveID::Course, path.values());
        if (ptr) {
            return ptr;
        }
    }
    return nullptr;
}

u32 ResMgr::GetMusicID() {
    return s_musicID;
}

u32 ResMgr::GetCourseID() {
    return s_courseID;
}

u32 ResMgr::s_musicID;
const MinimapConfig *ResMgr::s_minimapConfig;
