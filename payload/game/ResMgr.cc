#include "ResMgr.hh"

#include "game/CourseID.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartID.hh"
#include "game/KartLocale.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/SysDebug.hh"
#include "game/System.hh"

#include <common/Array.hh>
#include <common/Clock.hh>
#include <common/Log.hh>
#include <jsystem/JKRExpHeap.hh>
#include <jsystem/JKRMemArchive.hh>
#include <payload/Archive.hh>
#include <payload/ArchivePatcher.hh>
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

    void *courseHeapPtr = reinterpret_cast<void *>(0x800056c0);
    s_courseHeap = JKRExpHeap::Create(courseHeapPtr, 2560 * 1024, parentHeap, false);
    SysDebug::GetManager()->createHeapInfo(s_courseHeap, "Crs.arc");
}

const JKRHeap *ResMgr::GetCourseHeap() {
    return s_courseHeap;
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

void ResMgr::SwapForMenu(JKRHeap *heap) {
    if (s_swapState != SwapState::Menu) {
        s_mramArchiveSwapper->setIsSwappable("/item", true);
        s_mramArchiveSwapper->setIsSwappable("/objects", true);
        s_mramArchiveSwapper->swap(heap);
        s_swapState = SwapState::Menu;
    }
}

void ResMgr::SwapForRace(JKRHeap *heap) {
    SetKartIsSwappable(KartID::Mario, "mario");
    SetKartIsSwappable(KartID::Donkey, "dk");
    SetKartIsSwappable(KartID::Yoshi, "yoshi");
    SetKartIsSwappable(KartID::Nokonoko, "noko");
    SetKartIsSwappable(KartID::Peach, "peach");
    SetKartIsSwappable(KartID::BabyMario, "baby");
    SetKartIsSwappable(KartID::Wario, "wario");
    SetKartIsSwappable(KartID::Koopa, "koopa");
    SetKartIsSwappable(KartID::Luigi, "luigi");
    SetKartIsSwappable(KartID::Diddy, "diddy");
    SetKartIsSwappable(KartID::Catherine, "cathy");
    SetKartIsSwappable(KartID::Patapata, "pata");
    SetKartIsSwappable(KartID::Daisy, "daisy");
    SetKartIsSwappable(KartID::BabyLuigi, "bblui");
    SetKartIsSwappable(KartID::Waluigi, "walui");
    SetKartIsSwappable(KartID::KoopaJr, "jr");
    SetKartIsSwappable(KartID::Kinopio, "kpio");
    SetKartIsSwappable(KartID::Kinopico, "kpico");
    SetKartIsSwappable(KartID::Teresa, "teres");
    SetKartIsSwappable(KartID::Pakkun, "pakku");
    SetKartIsSwappable(KartID::Extra, "extra");
    s_mramArchiveSwapper->swap(heap);
    s_swapState = SwapState::Race;
}

JKRArchive *ResMgr::GetArchive(u32 archiveID) {
    return s_loaders[archiveID];
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

void ResMgr::LoadKeepData(void * /* userData */) {
    if (!(s_loadFlag & 1 << ArchiveID::ARAM)) {
        s_loadingFlag |= 1 << ArchiveID::ARAM;

        s_loaders[ArchiveID::ARAM] = JKRArchive::Mount("/ARAM.arc", JKRArchive::MountMode::Aram,
                System::GetAppHeap(), JKRArchive::MountDirection::Head);

        s_loadingFlag &= ~(1 << ArchiveID::ARAM);
        s_loadFlag |= 1 << ArchiveID::ARAM;
    }

    UniquePtr<u8[]> race2DArchive;
    u32 race2DArchiveSize;
    if (!(s_loadFlag & 1 << ArchiveID::MRAM)) {
        s_loadingFlag |= 1 << ArchiveID::MRAM;

        INFO("Loading MRAM.arc...");
        s_mramArchive = new (System::GetAppHeap(), 0x4) Archive(nullptr);
        for (;; Clock::WaitMilliseconds(200)) {
            race2DArchive.reset();

            u32 mramArchiveSize;
            UniquePtr<u8[]> mramArchive(reinterpret_cast<u8 *>(
                    FileLoader::Load("dvd:/MRAM.arc", System::GetAppHeap(), &mramArchiveSize)));
            if (!mramArchive.get()) {
                continue;
            }
            *s_mramArchive = Archive(mramArchive.get());
            if (!s_mramArchive->isValid(mramArchiveSize)) {
                continue;
            }

            Archive::Tree mramTree = s_mramArchive->getTree();
            const char *name;
            Archive::Dir dir(nullptr);
            Archive::Node node(nullptr);
            bool exists;
            if (!mramTree.search("/Race2D.arc", name, dir, node, exists)) {
                continue;
            }
            if (!exists || !node.isFile()) {
                continue;
            }
            race2DArchiveSize = node.getFileSize();
            race2DArchive.reset(new (System::GetAppHeap(), -0x20) u8[race2DArchiveSize]);
            if (!race2DArchive.get()) {
                continue;
            }
            memcpy(race2DArchive.get(), node.getFile(s_mramArchive->getFiles()), race2DArchiveSize);

            bool ownsMemory = true;
            if (ArchivePatcher::Patch("MRAM.arc", *s_mramArchive, mramArchiveSize,
                        System::GetAppHeap(), 0x20, ownsMemory)) {
                mramArchive.release();
                s_mramArchiveSwapper = new (System::GetAppHeap(), 0x4) ArchiveSwapper("MRAM.arc",
                        *s_mramArchive, System::GetAppHeap(), 384 * 1024, 448 * 1024);
                SwapForMenu(System::GetAppHeap());
                s_loaders[ArchiveID::MRAM] = JKRArchive::Mount(*s_mramArchive, mramArchiveSize,
                        JKRArchive::MountMode::Mem, System::GetAppHeap(),
                        JKRArchive::MountDirection::Head, false, false);
                break;
            }
        }
        INFO("Loaded MRAM.arc.");

        s_loadingFlag &= ~(1 << ArchiveID::MRAM);
        s_loadFlag |= 1 << ArchiveID::MRAM;
    }

    if (!(s_loadFlag & 1 << ArchiveID::Race2D)) {
        s_loadingFlag |= 1 << ArchiveID::Race2D;

        s_loaders[ArchiveID::Race2D] = JKRArchive::Mount(race2DArchive.release(), race2DArchiveSize,
                JKRArchive::MountMode::Mem, System::GetAppHeap(), JKRArchive::MountDirection::Head,
                true, true);

        s_loadingFlag &= ~(1 << ArchiveID::Race2D);
        s_loadFlag |= 1 << ArchiveID::Race2D;
    }

    if (!(s_loadFlag & 1 << ArchiveID::MRAMLoc)) {
        s_loadingFlag |= 1 << ArchiveID::MRAMLoc;

        Array<char, 64> path;
        snprintf(path.values(), path.count(), "/MRAM_Locale/%s/MRAMLoc.arc",
                KartLocale::GetLanguageName());
        s_loaders[ArchiveID::MRAMLoc] = JKRArchive::Mount(path.values(), JKRArchive::MountMode::Mem,
                System::GetAppHeap(), JKRArchive::MountDirection::Head);

        Kart2DCommon::Create(System::GetAppHeap());

        s_loadingFlag &= ~(1 << ArchiveID::MRAMLoc);
        s_loadFlag |= 1 << ArchiveID::MRAMLoc;
    }
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

void ResMgr::SetKartIsSwappable(u32 kartID, const char *kartName) {
    bool isSwappable = true;
    const RaceInfo &raceInfo = RaceInfo::Instance();
    for (s16 i = 0; i < raceInfo.getKartCount(); i++) {
        if (raceInfo.getKartInfo(i).getKartDB()->id == kartID) {
            isSwappable = false;
            break;
        }
    }
    Array<char, 32> path;
    snprintf(path.values(), path.count(), "/kart/%s_all", kartName);
    s_mramArchiveSwapper->setIsSwappable(path.values(), isSwappable);
}

Archive *ResMgr::s_mramArchive;
ArchiveSwapper *ResMgr::s_mramArchiveSwapper;
u32 ResMgr::s_swapState = SwapState::None;
const CourseManager::Course *ResMgr::s_course;
u32 ResMgr::s_musicID;
