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
    s_loaders[ArchiveID::System] = JKRArchive::Mount(DOL::BinarySectionStart(), parentHeap,
            JKRArchive::MountDirection::Head, true);

    size_t keepHeapSize = 0xf00000;
    void *keepHeap = MEM2Arena::Instance()->alloc(keepHeapSize, 0x4);
    s_keepHeap = JKRExpHeap::Create(keepHeap, keepHeapSize, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_keepHeap, "MRAM.arc");

    s_courseHeap = JKRSolidHeap::Create(0x280000, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
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
    CourseManager::Instance()->freeAll();
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
    s_loaders[ArchiveID::Course] =
            JKRArchive::Mount(path.values(), s_courseHeap, JKRArchive::MountDirection::Head, false);

    s_mountCourseID = s_courseID;
    s_mountCourseOrder = s_courseOrder;
    s_loadingFlag &= ~(1 << ArchiveID::Course);
    s_loadFlag |= 1 << ArchiveID::Course;
}

void ResMgr::LoadExtendedCourseData(void * /* userData */) {
    s_loadingFlag |= 1 << ArchiveID::Course;

    s_courseName = s_course->loadLogo();
    s_staffGhost = s_course->loadStaffGhost();
    u32 raceLevel = RaceInfo::Instance().getRaceLevel();
    void *courseArchive = s_course->loadCourse(s_courseOrder, raceLevel);
    s_loaders[ArchiveID::Course] =
            JKRArchive::Mount(courseArchive, s_courseHeap, JKRArchive::MountDirection::Head, false);

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
