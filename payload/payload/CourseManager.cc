#include "CourseManager.hh"

#include "payload/FileLoader.hh"
#include "payload/MinimapConfigReader.hh"
#include "payload/SZSCourseHasher.hh"
#include "payload/SZSCourseLoader.hh"
#include "payload/ZIPFile.hh"

#include <common/Arena.hh>
#include <common/DCache.hh>
#include <common/Log.hh>
#include <game/CourseID.hh>
#include <game/ResMgr.hh>
#include <jsystem/JKRExpHeap.hh>
extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
}

CourseManager::Pack::Pack(Ring<u32, MaxCourseCount> courseIndices)
    : m_courseIndices(courseIndices) {}

CourseManager::Pack::~Pack() {}

const Ring<u32, CourseManager::MaxCourseCount> &CourseManager::Pack::courseIndices() const {
    return m_courseIndices;
}

Ring<u32, CourseManager::MaxCourseCount> &CourseManager::Pack::courseIndices() {
    return m_courseIndices;
}

CourseManager::Course::Course(Array<u8, 32> archiveHash) : m_archiveHash(archiveHash) {}

CourseManager::Course::~Course() {}

Array<u8, 32> CourseManager::Course::archiveHash() const {
    return m_archiveHash;
}

CourseManager::DefaultPack::DefaultPack(Ring<u32, MaxCourseCount> courseIndices,
        Array<char, 32> name)
    : Pack(courseIndices), m_name(name) {}

CourseManager::DefaultPack::~DefaultPack() {}

const char *CourseManager::DefaultPack::name() const {
    Array<char, 32> path;
    snprintf(path.values(), path.count(), "/packnames/%s.txt", m_name.values());
    char *name = reinterpret_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAM, path.values()));
    u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAM, name);
    name[size - 1] = '\0';
    return name;
}

const char *CourseManager::DefaultPack::author() const {
    return nullptr;
}

const char *CourseManager::DefaultPack::version() const {
    return nullptr;
}

CourseManager::CustomPack::CustomPack(Ring<u32, MaxCourseCount> courseIndices,
        Array<char, INIFieldSize> name, Array<char, INIFieldSize> author,
        Array<char, INIFieldSize> version)
    : Pack(courseIndices), m_name(name), m_author(author), m_version(version) {}

CourseManager::CustomPack::~CustomPack() {}

const char *CourseManager::CustomPack::name() const {
    return m_name.values();
}

const char *CourseManager::CustomPack::author() const {
    return m_author.values();
}

const char *CourseManager::CustomPack::version() const {
    return m_version.values();
}

CourseManager::DefaultCourse::DefaultCourse(Array<u8, 32> archiveHash, u32 courseID,
        const char *thumbnail, const char *nameImage)
    : Course(archiveHash), m_courseID(courseID), m_thumbnail(thumbnail), m_nameImage(nameImage) {}

CourseManager::DefaultCourse::~DefaultCourse() {}

u32 CourseManager::DefaultCourse::musicID() const {
    return m_courseID;
}

const char *CourseManager::DefaultCourse::name() const {
    Array<char, 32> path;
    snprintf(path.values(), path.count(), "/coursenames/%02x.txt", m_courseID);
    char *name = reinterpret_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAM, path.values()));
    u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAM, name);
    name[size - 1] = '\0';
    return name;
}

const char *CourseManager::DefaultCourse::author() const {
    return nullptr;
}

const char *CourseManager::DefaultCourse::version() const {
    return nullptr;
}

const MinimapConfig *CourseManager::DefaultCourse::minimapConfig() const {
    return nullptr;
}

void *CourseManager::DefaultCourse::thumbnail() const {
    return JKRFileLoader::GetGlbResource(m_thumbnail, nullptr);
}

void *CourseManager::DefaultCourse::nameImage() const {
    return JKRFileLoader::GetGlbResource(m_nameImage, nullptr);
}

void *CourseManager::DefaultCourse::loadLogo(JKRHeap *heap) const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    const char *languageName = KartLocale::GetLanguageName();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/CourseName/%s/%s_name.bti", languageName, base);
    return FileLoader::Load(path.values(), heap);
}

void *CourseManager::DefaultCourse::loadStaffGhost(JKRHeap *heap) const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/StaffGhosts/%s.ght", base);
    return FileLoader::Load(path.values(), heap);
}

void *CourseManager::DefaultCourse::loadCourse(u32 courseOrder, u32 raceLevel,
        JKRHeap *heap) const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    const char *suffix = courseOrder == 1 ? "L" : "";
    Array<char, 256> path;
    if (m_courseID == 0x24 && raceLevel != 0) {
        snprintf(path.values(), path.count(), "dvd:/Course/Luigi2%s.arc", suffix);
    } else {
        snprintf(path.values(), path.count(), "dvd:/Course/%s%s.arc", base, suffix);
    }
    return FileLoader::Load(path.values(), heap);
}

bool CourseManager::DefaultCourse::isDefault() const {
    return true;
}

bool CourseManager::DefaultCourse::isCustom() const {
    return false;
}

CourseManager::CustomCourse::CustomCourse(Array<u8, 32> archiveHash, u32 musicID,
        Array<char, INIFieldSize> name, Array<char, INIFieldSize> author,
        Array<char, INIFieldSize> version, Optional<MinimapConfig> minimapConfig, u8 *thumbnail,
        u8 *nameImage, Array<char, 256> path, Array<char, 128> prefix)
    : Course(archiveHash), m_musicID(musicID), m_name(name), m_author(author), m_version(version),
      m_minimapConfig(minimapConfig), m_thumbnail(thumbnail), m_nameImage(nameImage), m_path(path),
      m_prefix(prefix) {}

CourseManager::CustomCourse::~CustomCourse() {}

u32 CourseManager::CustomCourse::musicID() const {
    return m_musicID;
}

const char *CourseManager::CustomCourse::name() const {
    return m_name.values();
}

const char *CourseManager::CustomCourse::author() const {
    return m_author.values();
}

const char *CourseManager::CustomCourse::version() const {
    return m_version.values();
}

const MinimapConfig *CourseManager::CustomCourse::minimapConfig() const {
    return m_minimapConfig.get();
}

void *CourseManager::CustomCourse::thumbnail() const {
    return m_thumbnail.get();
}

void *CourseManager::CustomCourse::nameImage() const {
    return m_nameImage.get();
}

void *CourseManager::CustomCourse::loadLogo(JKRHeap *heap) const {
    Array<char, 256> logoPrefix;
    snprintf(logoPrefix.values(), logoPrefix.count(), "/%scourse_images/", m_prefix.values());
    u32 logoSize;
    void *logo = s_instance->loadLocalizedFile(m_path.values(), logoPrefix.values(),
            "/track_big_logo.bti", heap, &logoSize);
    if (logo) {
        DCache::Flush(logo, logoSize);
    }
    return logo;
}

void *CourseManager::CustomCourse::loadStaffGhost(JKRHeap *heap) const {
    Array<char, 256> staffGhostPath;
    snprintf(staffGhostPath.values(), staffGhostPath.count(), "/%sstaffghost.ght",
            m_prefix.values());
    return s_instance->loadFile(m_path.values(), staffGhostPath.values(), heap);
}

void *CourseManager::CustomCourse::loadCourse(u32 /* courseOrder */, u32 /* raceLevel */,
        JKRHeap *heap) const {
    Array<char, 256> coursePath;
    snprintf(coursePath.values(), coursePath.count(), "/%strack.arc", m_prefix.values());
    u32 courseSize;
    void *course =
            s_instance->loadCourseFile(m_path.values(), coursePath.values(), heap, &courseSize);
    if (course) {
        DCache::Flush(course, courseSize);
    }
    return course;
}

bool CourseManager::CustomCourse::isDefault() const {
    return false;
}

bool CourseManager::CustomCourse::isCustom() const {
    return true;
}

void CourseManager::start() {
    size_t heapSize = 0x500000;
    void *heapPtr = MEM2Arena::Instance()->alloc(heapSize, 0x4);
    m_heap = JKRExpHeap::Create(heapPtr, heapSize, JKRHeap::GetRootHeap(), false);
    StorageScanner::start();
}

u32 CourseManager::racePackCount() const {
    return m_racePacks.count();
}

u32 CourseManager::battlePackCount() const {
    return m_battlePacks.count();
}

const CourseManager::Pack &CourseManager::racePack(u32 index) const {
    return *m_racePacks[index];
}

const CourseManager::Pack &CourseManager::battlePack(u32 index) const {
    return *m_battlePacks[index];
}

u32 CourseManager::raceCourseCount(u32 packIndex) const {
    return m_racePacks[packIndex]->courseIndices().count();
}

u32 CourseManager::battleCourseCount(u32 packIndex) const {
    return m_battlePacks[packIndex]->courseIndices().count();
}

const CourseManager::Course &CourseManager::raceCourse(u32 packIndex, u32 index) const {
    return *m_raceCourses[m_racePacks[packIndex]->courseIndices()[index]];
}

const CourseManager::Course &CourseManager::battleCourse(u32 packIndex, u32 index) const {
    return *m_battleCourses[m_battlePacks[packIndex]->courseIndices()[index]];
}

void CourseManager::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x4) CourseManager;
}

CourseManager *CourseManager::Instance() {
    return s_instance;
}

CourseManager::CourseManager() {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 26,
            0);
}

OSThread &CourseManager::thread() {
    return m_thread;
}

void CourseManager::process() {
    m_raceCourses.reset();
    m_battleCourses.reset();
    m_racePacks.reset();
    m_battlePacks.reset();
    addDefaultRaceCourses();
    addDefaultBattleCourses();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/ddd/courses");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    Ring<u32, MaxCourseCount> raceCourseIndices;
    Ring<u32, MaxCourseCount> battleCourseIndices;
    addCustomPacksAndCourses(path, nodeInfo, raceCourseIndices, battleCourseIndices);
    sortRacePacksByName();
    sortBattlePacksByName();
    addDefaultRacePacks();
    addDefaultBattlePacks();
    sortRacePackCoursesByName();
    sortBattlePackCoursesByName();
}

void CourseManager::addDefaultRaceCourses() {
    Array<Array<u8, 32>, DefaultRaceCourseCount> archiveHashes;
    Array<u32, DefaultRaceCourseCount> courseIDs;
    Array<const char *, DefaultRaceCourseCount> thumbnails;
    Array<const char *, DefaultRaceCourseCount> nameImages;

    archiveHashes[0] = (u8[32]){0xde, 0x46, 0xd0, 0x58, 0xe3, 0x2a, 0x4e, 0xa7, 0x85, 0xda, 0xd6,
            0xa0, 0x41, 0xf9, 0xca, 0x47, 0x4b, 0x49, 0x65, 0x33, 0x1c, 0xca, 0xb0, 0x3c, 0xbf,
            0x14, 0x28, 0xc9, 0xd5, 0xdd, 0x08, 0xa0};
    courseIDs[0] = CourseID::BabyLuigi;
    thumbnails[0] = "COP_BABY_PARK.bti";
    nameImages[0] = "CoName_BABY_PARK.bti";

    archiveHashes[1] = (u8[32]){0x45, 0x55, 0x94, 0x11, 0x4e, 0xf3, 0xd8, 0x22, 0xcc, 0x67, 0x5d,
            0x01, 0x02, 0x7f, 0x34, 0xa0, 0x43, 0xd0, 0x8f, 0xda, 0x29, 0x93, 0x4a, 0x75, 0xd9,
            0x5d, 0x3d, 0x68, 0x19, 0x34, 0x19, 0xdb};
    courseIDs[1] = CourseID::Peach;
    thumbnails[1] = "COP_PEACH_BEACH.bti";
    nameImages[1] = "CoName_PEACH_BEACH.bti";

    archiveHashes[2] = (u8[32]){0x1d, 0xa8, 0x67, 0x10, 0x53, 0x66, 0x61, 0x79, 0xd4, 0x0d, 0x0a,
            0xcd, 0xa7, 0x29, 0x1d, 0xc0, 0x22, 0xd1, 0xf5, 0xba, 0x5c, 0xe6, 0xf3, 0x1f, 0x8e,
            0x70, 0x43, 0x62, 0xaf, 0x8b, 0x0e, 0x9d};
    courseIDs[2] = CourseID::Daisy;
    thumbnails[2] = "COP_DAISY_SHIP.bti";
    nameImages[2] = "CoName_DAISY_SHIP.bti";

    archiveHashes[3] = (u8[32]){0xed, 0xc2, 0x0f, 0x19, 0xd1, 0xa0, 0xde, 0xed, 0x3e, 0xac, 0x69,
            0x0b, 0xf9, 0x61, 0x77, 0xe7, 0xce, 0x8a, 0x56, 0xb4, 0xf8, 0x48, 0xaa, 0xcd, 0x63,
            0x12, 0x68, 0xbd, 0x69, 0x12, 0x09, 0x74};
    courseIDs[3] = CourseID::Luigi;
    thumbnails[3] = "COP_LUIGI_CIRCUIT.bti";
    nameImages[3] = "CoName_LUIGI_CIRCUIT.bti";

    archiveHashes[4] = (u8[32]){0x93, 0xce, 0x7a, 0x45, 0x16, 0x6d, 0xd0, 0x61, 0xcb, 0xb5, 0xb0,
            0x06, 0xca, 0xa0, 0x11, 0xc0, 0x27, 0xa6, 0x47, 0x9e, 0x37, 0x05, 0x35, 0x36, 0x51,
            0x64, 0x4b, 0x7f, 0x8f, 0xa1, 0x63, 0xd3};
    courseIDs[4] = CourseID::Mario;
    thumbnails[4] = "COP_MARIO_CIRCUIT.bti";
    nameImages[4] = "CoName_MARIO_CIRCUIT.bti";

    archiveHashes[5] = (u8[32]){0xb6, 0x76, 0xa5, 0xfa, 0xb8, 0x8e, 0xa5, 0x37, 0xe4, 0xb5, 0x2d,
            0x98, 0xe2, 0x80, 0xe8, 0x25, 0xbd, 0x18, 0xd2, 0xf4, 0xa1, 0x76, 0xdb, 0x80, 0xa1,
            0x4a, 0x71, 0x8a, 0x9d, 0xce, 0xce, 0xab};
    courseIDs[5] = CourseID::Yoshi;
    thumbnails[5] = "COP_YOSHI_CIRCUIT.bti";
    nameImages[5] = "CoName_YOSHI_CIRCUIT.bti";

    archiveHashes[6] = (u8[32]){0x49, 0xd2, 0xb6, 0x44, 0x32, 0xf3, 0x83, 0x8a, 0x9c, 0x45, 0x4e,
            0x8d, 0x9f, 0xdc, 0xbf, 0x5b, 0x5f, 0xe2, 0x47, 0x94, 0x97, 0x5f, 0xb6, 0xa7, 0xf2,
            0xe3, 0xa4, 0xfe, 0x5a, 0xc4, 0xe3, 0xbc};
    courseIDs[6] = CourseID::Nokonoko;
    thumbnails[6] = "COP_KINOKO_BRIDGE.bti";
    nameImages[6] = "CoName_KINOKO_BRIDGE.bti";

    archiveHashes[7] = (u8[32]){0x18, 0x7a, 0x5f, 0xf2, 0xca, 0x4e, 0x90, 0xe8, 0xc0, 0x1b, 0x78,
            0x21, 0x39, 0xce, 0xcf, 0x52, 0xbe, 0x55, 0xdf, 0x15, 0x68, 0x48, 0xff, 0x02, 0x72,
            0x5a, 0x48, 0x17, 0xa9, 0xd0, 0xa7, 0xea};
    courseIDs[7] = CourseID::Patapata;
    thumbnails[7] = "COP_KONOKO_CITY.bti";
    nameImages[7] = "CoName_KINOKO_CITY.bti";

    archiveHashes[8] = (u8[32]){0x71, 0x4b, 0xdb, 0x4c, 0x40, 0xaf, 0x00, 0x05, 0x72, 0xb3, 0xa3,
            0x14, 0xf8, 0x33, 0xce, 0x4d, 0x2c, 0xf7, 0x55, 0xf1, 0xfa, 0x8e, 0xe7, 0x58, 0xcc,
            0xb0, 0xb7, 0xcb, 0xc5, 0x75, 0xd3, 0x15};
    courseIDs[8] = CourseID::Waluigi;
    thumbnails[8] = "COP_WALUIGI_STADIUM.bti";
    nameImages[8] = "CoName_WALUIGI_STADIUM.bti";

    archiveHashes[9] = (u8[32]){0x14, 0x5f, 0xa1, 0x5b, 0xe2, 0xf9, 0xb9, 0x7c, 0x01, 0xe3, 0x7a,
            0x02, 0x72, 0x87, 0x68, 0x29, 0xb5, 0x71, 0x09, 0xf4, 0x36, 0xe3, 0xb2, 0x60, 0x6c,
            0x39, 0x48, 0xee, 0x03, 0xd7, 0x55, 0x10};
    courseIDs[9] = CourseID::Wario;
    thumbnails[9] = "COP_WARIO_COLOSSEUM.bti";
    nameImages[9] = "CoName_WARIO_COLOSSEUM.bti";

    archiveHashes[10] = (u8[32]){0x56, 0x79, 0x52, 0x42, 0xfc, 0x77, 0x75, 0x86, 0xad, 0xa3, 0xd4,
            0x99, 0xf2, 0xae, 0xf2, 0x7e, 0xd8, 0x1e, 0x9c, 0xfa, 0x12, 0x1d, 0xdb, 0xc3, 0xc5,
            0x2d, 0x2e, 0xf5, 0xb0, 0x34, 0x11, 0x1d};
    courseIDs[10] = CourseID::Diddy;
    thumbnails[10] = "COP_DINO_DINO_JUNGLE.bti";
    nameImages[10] = "CoName_DINO_DINO_JUNGLE.bti";

    archiveHashes[11] = (u8[32]){0x6b, 0xf8, 0x62, 0x29, 0xe9, 0x8d, 0x71, 0xd7, 0x4d, 0xef, 0xde,
            0x1d, 0x3b, 0x17, 0x4b, 0xdc, 0x42, 0x02, 0x28, 0xbc, 0x6e, 0xf0, 0xc8, 0x41, 0xce,
            0x73, 0x29, 0x93, 0x06, 0x66, 0xd5, 0xae};
    courseIDs[11] = CourseID::Donkey;
    thumbnails[11] = "COP_DK_MOUNTAIN.bti";
    nameImages[11] = "CoName_DK_MOUNTAIN.bti";

    archiveHashes[12] = (u8[32]){0xfc, 0xe2, 0x97, 0x0d, 0xa8, 0x10, 0xc5, 0xc5, 0x6a, 0x5f, 0x22,
            0x7d, 0x48, 0x88, 0xba, 0xd6, 0x6e, 0x87, 0xc1, 0x03, 0x90, 0x39, 0x87, 0x33, 0x8f,
            0xf3, 0xf4, 0x6d, 0x2c, 0xbf, 0x13, 0x56};
    courseIDs[12] = CourseID::Koopa;
    thumbnails[12] = "COP_BOWSER_CASTLE.bti";
    nameImages[12] = "CoName_BOWSER_CASTLE.bti";

    archiveHashes[13] = (u8[32]){0x48, 0xdf, 0x40, 0x53, 0xf4, 0x6b, 0xaf, 0x42, 0x4f, 0x6d, 0xf6,
            0x7c, 0xa9, 0xe0, 0x18, 0x66, 0xe7, 0x8f, 0x67, 0xda, 0x1f, 0x72, 0xe8, 0x45, 0x60,
            0xcb, 0x43, 0xc9, 0xc1, 0x73, 0x3f, 0xe0};
    courseIDs[13] = CourseID::Rainbow;
    thumbnails[13] = "COP_RAINBOW_ROAD.bti";
    nameImages[13] = "CoName_RAINBOW_ROAD.bti";

    archiveHashes[14] = (u8[32]){0x72, 0x8e, 0x8f, 0x43, 0x44, 0x3f, 0xdd, 0x1e, 0x1d, 0x2c, 0xd8,
            0x82, 0x96, 0x10, 0x05, 0x42, 0x9b, 0x91, 0x3f, 0x7f, 0x11, 0x2c, 0x3c, 0xd5, 0x57,
            0x78, 0x71, 0x9d, 0x7b, 0x9f, 0xb9, 0xbe};
    courseIDs[14] = CourseID::Desert;
    thumbnails[14] = "COP_KARA_KARA_DESERT.bti";
    nameImages[14] = "CoName_KARA_KARA_DESERT.bti";

    archiveHashes[15] = (u8[32]){0xde, 0xa9, 0x65, 0x9a, 0x16, 0x71, 0xa4, 0x9d, 0x0c, 0xb2, 0xe6,
            0x06, 0x66, 0xe0, 0x28, 0x64, 0xae, 0x20, 0x07, 0xe8, 0xf0, 0x3d, 0x23, 0x7a, 0xd7,
            0x4e, 0x7b, 0xe3, 0xdf, 0xa5, 0x8d, 0x5e};
    courseIDs[15] = CourseID::Snow;
    thumbnails[15] = "COP_SHERBET_LAND.bti";
    nameImages[15] = "CoName_SHERBET_LAND.bti";

    for (u32 i = 0; i < DefaultRaceCourseCount; i++) {
        DEBUG("Adding default race course 0x%02x...", courseIDs[i]);
        m_raceCourses.pushBack();
        Course *course = new (m_heap, 0x4)
                DefaultCourse(archiveHashes[i], courseIDs[i], thumbnails[i], nameImages[i]);
        m_raceCourses.back()->reset(course);
    }
}

void CourseManager::addDefaultBattleCourses() {
    Array<Array<u8, 32>, DefaultBattleCourseCount> archiveHashes;
    Array<u32, DefaultBattleCourseCount> courseIDs;
    Array<const char *, DefaultBattleCourseCount> thumbnails;
    Array<const char *, DefaultBattleCourseCount> nameImages;

    archiveHashes[0] = (u8[32]){0x41, 0x20, 0x0c, 0x6c, 0x01, 0xa1, 0xe8, 0x6d, 0xdf, 0xf1, 0xa3,
            0xf0, 0x53, 0xfa, 0xd7, 0x7d, 0x5f, 0x87, 0xbe, 0xa7, 0x3a, 0xdf, 0xe1, 0x0e, 0x48,
            0x39, 0x6d, 0xcc, 0x29, 0x58, 0xff, 0xb6};
    courseIDs[0] = CourseID::Mini1;
    thumbnails[0] = "BattleMapSnap6.bti";
    nameImages[0] = "Mozi_Map6.bti";

    archiveHashes[1] = (u8[32]){0xea, 0x3b, 0x9a, 0x47, 0x87, 0x82, 0x8c, 0x1a, 0xf3, 0xbf, 0xcd,
            0x39, 0xdc, 0x51, 0x60, 0xaa, 0x6a, 0x2b, 0x67, 0x6c, 0x62, 0x94, 0x30, 0x33, 0x3f,
            0xee, 0xce, 0xf7, 0xe7, 0xdd, 0x7c, 0x90};
    courseIDs[1] = CourseID::Mini2;
    thumbnails[1] = "BattleMapSnap3.bti";
    nameImages[1] = "Mozi_Map3.bti";

    archiveHashes[2] = (u8[32]){0x29, 0xab, 0xaf, 0x57, 0xde, 0x86, 0x71, 0x15, 0xab, 0x69, 0xaf,
            0x1b, 0x3d, 0x5b, 0x75, 0x4b, 0x26, 0x0b, 0x84, 0x8d, 0xa1, 0xdf, 0x39, 0x73, 0xd7,
            0x27, 0x2c, 0x7d, 0x55, 0xf1, 0xd6, 0x5d};
    courseIDs[2] = CourseID::Mini3;
    thumbnails[2] = "BattleMapSnap2.bti";
    nameImages[2] = "Mozi_Map2.bti";

    archiveHashes[3] = (u8[32]){0xac, 0xb4, 0x4e, 0xa2, 0x23, 0x9a, 0xb0, 0x0c, 0xd2, 0xd8, 0x39,
            0x04, 0x07, 0xfe, 0xac, 0x7e, 0x07, 0x09, 0xbd, 0x4f, 0x18, 0xaa, 0x7f, 0x08, 0x5d,
            0x1c, 0x58, 0xb7, 0x8d, 0xa6, 0x98, 0x4b};
    courseIDs[3] = CourseID::Mini5;
    thumbnails[3] = "BattleMapSnap5.bti";
    nameImages[3] = "Mozi_Map5.bti";

    archiveHashes[4] = (u8[32]){0xf8, 0x0d, 0xa9, 0xd7, 0xc2, 0xc8, 0x11, 0x31, 0x51, 0x24, 0x87,
            0x2c, 0x78, 0xd7, 0xd4, 0xd8, 0xb8, 0xc6, 0xf9, 0x2b, 0xcc, 0x5d, 0x89, 0xbb, 0x90,
            0x5d, 0x3b, 0x6d, 0xe9, 0xe9, 0x1f, 0x77};
    courseIDs[4] = CourseID::Mini7;
    thumbnails[4] = "BattleMapSnap1.bti";
    nameImages[4] = "Mozi_Map1.bti";

    archiveHashes[5] = (u8[32]){0x02, 0x29, 0x79, 0x86, 0x90, 0xa4, 0x94, 0x73, 0x1a, 0x59, 0xdf,
            0x70, 0xa7, 0xda, 0x56, 0x44, 0xa1, 0x30, 0x01, 0xcd, 0x45, 0x5f, 0x47, 0x35, 0x17,
            0x43, 0x92, 0x2c, 0xa3, 0xe2, 0x33, 0x2e};
    courseIDs[5] = CourseID::Mini8;
    thumbnails[5] = "BattleMapSnap4.bti";
    nameImages[5] = "Mozi_Map4.bti";

    for (u32 i = 0; i < DefaultBattleCourseCount; i++) {
        DEBUG("Adding default battle course 0x%02x...", courseIDs[i]);
        m_battleCourses.pushBack();
        Course *course = new (m_heap, 0x4)
                DefaultCourse(archiveHashes[i], courseIDs[i], thumbnails[i], nameImages[i]);
        m_battleCourses.back()->reset(course);
    }
}

void CourseManager::addCustomPacksAndCourses(Array<char, 256> &path, Storage::NodeInfo &nodeInfo,
        Ring<u32, MaxCourseCount> &raceCourseIndices,
        Ring<u32, MaxCourseCount> &battleCourseIndices) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        if (nodeInfo.type == Storage::NodeType::Dir) {
            snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
            addCustomPacksAndCourses(path, nodeInfo, raceCourseIndices, battleCourseIndices);
        }
    }
    path[length] = '\0';
    raceCourseIndices.reset();
    battleCourseIndices.reset();
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        if (nodeInfo.type == Storage::NodeType::File) {
            snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
            addCustomCourse(path, raceCourseIndices, battleCourseIndices);
        }
    }
    path[length] = '\0';
    addCustomRacePack(path, raceCourseIndices);
    addCustomBattlePack(path, battleCourseIndices);
}

void CourseManager::addCustomCourse(const Array<char, 256> &path,
        Ring<u32, MaxCourseCount> &raceCourseIndices,
        Ring<u32, MaxCourseCount> &battleCourseIndices) {
    ZIPFile zipFile(path.values());
    if (!zipFile.ok()) {
        return;
    }

    Array<char, 128> prefix;
    INI::Stream iniStream;
    {
        ZIPFile::Reader reader(zipFile, "trackinfo.ini");
        if (!reader.ok()) {
            return;
        }

        prefix = reader.cdNode()->path;
        u32 prefixLength = strlen(reader.cdNode()->path.values()) - strlen("trackinfo.ini");
        prefix[prefixLength] = '\0';

        iniStream.iniSize = *reader.size();
        iniStream.ini.reset(new (m_heap, -0x4) u8[iniStream.iniSize]);
        if (!iniStream.ini.get()) {
            return;
        }
        for (u32 offset = 0; offset < iniStream.iniSize;) {
            const u8 *chunk;
            u32 chunkSize;
            if (!reader.read(chunk, chunkSize)) {
                return;
            }
            memcpy(iniStream.ini.get() + offset, chunk, chunkSize);
            offset += chunkSize;
        }
    }
    iniStream.iniOffset = 0x0;

    CourseINI courseINI;
    Array<INI::Field, 5> iniFields;
    iniFields[0] = (INI::Field){"trackname", &courseINI.fallbackName};
    iniFields[1] = (INI::Field){"author", &courseINI.fallbackAuthor};
    iniFields[2] = (INI::Field){"version", &courseINI.version};
    iniFields[3] = (INI::Field){"replaces", &courseINI.defaultCourseName};
    iniFields[4] = (INI::Field){"auxiliary_audio_track", &courseINI.defaultMusicName};
    Array<INI::LocalizedField, 2> localizedINIFields;
    localizedINIFields[0] = (INI::LocalizedField){"trackname_", &courseINI.localizedNames};
    localizedINIFields[1] = (INI::LocalizedField){"author_", &courseINI.localizedAuthors};

    INI ini(iniStream, iniFields.count(), iniFields.values(), localizedINIFields.count(),
            localizedINIFields.values());
    if (!ini.read()) {
        return;
    }

    Array<char, INIFieldSize> &name =
            getLocalizedEntry(courseINI.localizedNames, courseINI.fallbackName);
    if (strlen(name.values()) == 0) {
        return;
    }
    Array<char, INIFieldSize> &author =
            getLocalizedEntry(courseINI.localizedAuthors, courseINI.fallbackAuthor);
    Array<char, INIFieldSize> &version = courseINI.version;

    u32 courseID;
    if (!GetDefaultCourseID(courseINI.defaultCourseName.values(), courseID)) {
        return;
    }

    u32 musicID;
    if (!GetDefaultCourseID(courseINI.defaultMusicName.values(), musicID)) {
        musicID = courseID;
    }

    Array<char, 256> minimapJSONPath;
    snprintf(minimapJSONPath.values(), minimapJSONPath.count(), "/%sminimap.json", prefix.values());
    Optional<MinimapConfig> minimapConfig =
            MinimapConfigReader::Read(zipFile, minimapJSONPath.values());

    Array<char, 256> thumbnailPrefix;
    snprintf(thumbnailPrefix.values(), thumbnailPrefix.count(), "/%scourse_images/",
            prefix.values());
    u32 thumbnailSize;
    UniquePtr<u8[]> thumbnail(reinterpret_cast<u8 *>(loadLocalizedFile(zipFile,
            thumbnailPrefix.values(), "/track_image.bti", m_heap, &thumbnailSize)));
    if (!thumbnail.get() || thumbnailSize < 0x20) {
        return;
    }
    DCache::Flush(thumbnail.get(), thumbnailSize);

    Array<char, 256> nameImagePrefix;
    snprintf(nameImagePrefix.values(), nameImagePrefix.count(), "/%scourse_images/",
            prefix.values());
    u32 nameImageSize;
    UniquePtr<u8[]> nameImage(reinterpret_cast<u8 *>(loadLocalizedFile(zipFile,
            nameImagePrefix.values(), "/track_name.bti", m_heap, &nameImageSize)));
    if (!nameImage.get() || nameImageSize < 0x20) {
        return;
    }
    DCache::Flush(nameImage.get(), nameImageSize);

    Array<u8, 32> archiveHash;
    Array<char, 256> hashPath;
    snprintf(hashPath.values(), hashPath.count(), "/%shash.bin", prefix.values());
    u32 hashSize;
    UniquePtr<u8[]> hash(
            reinterpret_cast<u8 *>(loadFile(zipFile, hashPath.values(), m_heap, &hashSize)));
    if (hash.get() && hashSize == archiveHash.count()) {
        memcpy(archiveHash.values(), hash.get(), archiveHash.count());
    } else {
        Array<char, 256> coursePath;
        snprintf(coursePath.values(), coursePath.count(), "/%strack.arc", prefix.values());
        if (!hashCourseFile(zipFile, coursePath.values(), archiveHash)) {
            return;
        }

        ZIPFile::Writer writer(zipFile, hashPath.values(), archiveHash.count());
        if (!writer.write(archiveHash.values(), archiveHash.count())) {
            return;
        }
    }

    Ring<UniquePtr<Course>, MaxCourseCount> *courses = nullptr;
    Ring<u32, MaxCourseCount> *courseIndices = nullptr;
    const char *type = nullptr;
    switch (courseID) {
    case CourseID::BabyLuigi:
    case CourseID::Peach:
    case CourseID::Daisy:
    case CourseID::Luigi:
    case CourseID::Mario:
    case CourseID::Yoshi:
    case CourseID::Nokonoko:
    case CourseID::Patapata:
    case CourseID::Waluigi:
    case CourseID::Wario:
    case CourseID::Diddy:
    case CourseID::Donkey:
    case CourseID::Koopa:
    case CourseID::Rainbow:
    case CourseID::Desert:
    case CourseID::Snow:
        courses = &m_raceCourses;
        courseIndices = &raceCourseIndices;
        type = "race";
        break;
    case CourseID::Mini1:
    case CourseID::Mini2:
    case CourseID::Mini3:
    case CourseID::Mini5:
    case CourseID::Mini7:
    case CourseID::Mini8:
        courses = &m_battleCourses;
        courseIndices = &battleCourseIndices;
        type = "battle";
        break;
    default:
        return;
    }

    for (u32 i = 0; i < courses->count(); i++) {
        if (!memcmp(archiveHash.values(), (*courses)[i]->archiveHash().values(),
                    archiveHash.count())) {
            courseIndices->pushBack(i);
            return;
        }
    }

    DEBUG("Adding custom %s course %s...", type, path.values());
    courseIndices->pushBack(courses->count());
    courses->pushBack();
    Course *course = new (m_heap, 0x4) CustomCourse(archiveHash, musicID, name, author, version,
            minimapConfig, thumbnail.release(), nameImage.release(), path, prefix);
    courses->back()->reset(course);
}

void CourseManager::addCustomRacePack(const Array<char, 256> &path,
        Ring<u32, MaxCourseCount> &courseIndices) {
    addCustomPack(path, courseIndices, 0, DefaultRaceCourseCount, m_racePacks, "race");
}

void CourseManager::addCustomBattlePack(const Array<char, 256> &path,
        Ring<u32, MaxCourseCount> &courseIndices) {
    addCustomPack(path, courseIndices, DefaultRaceCourseCount, DefaultBattleCourseCount,
            m_battlePacks, "battle");
}

void CourseManager::addCustomPack(const Array<char, 256> &path,
        Ring<u32, MaxCourseCount> &courseIndices, u32 defaultCourseOffset, u32 defaultCourseCount,
        Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *type) {
    Array<char, 256> iniPath;
    snprintf(iniPath.values(), iniPath.count(), "%s/packinfo.ini", path.values());
    INI::Stream iniStream;
    iniStream.ini.reset(
            reinterpret_cast<u8 *>(FileLoader::Load(iniPath.values(), m_heap, &iniStream.iniSize)));
    if (!iniStream.ini.get()) {
        return;
    }
    iniStream.iniOffset = 0x0;

    PackINI packINI;
    Array<INI::Field, 4> iniFields;
    iniFields[0] = (INI::Field){"packname", &packINI.fallbackName};
    iniFields[1] = (INI::Field){"author", &packINI.fallbackAuthor};
    iniFields[2] = (INI::Field){"version", &packINI.version};
    iniFields[3] = (INI::Field){"vanilla_tracks", &packINI.defaultCourses};
    Array<INI::LocalizedField, 2> localizedINIFields;
    localizedINIFields[0] = (INI::LocalizedField){"packname_", &packINI.localizedNames};
    localizedINIFields[1] = (INI::LocalizedField){"author_", &packINI.localizedAuthors};

    INI ini(iniStream, iniFields.count(), iniFields.values(), localizedINIFields.count(),
            localizedINIFields.values());
    if (!ini.read()) {
        return;
    }

    Array<char, INIFieldSize> &name =
            getLocalizedEntry(packINI.localizedNames, packINI.fallbackName);
    if (strlen(name.values()) == 0) {
        return;
    }
    Array<char, INIFieldSize> &author =
            getLocalizedEntry(packINI.localizedAuthors, packINI.fallbackAuthor);
    Array<char, INIFieldSize> &version = packINI.version;

    const char *c = packINI.defaultCourses.values();
    for (u32 i = 0; i < defaultCourseOffset && *c; i++, c++) {}
    for (u32 i = 0; i < defaultCourseCount && *c; i++, c++) {
        if (*c == 'Y' || *c == 'y' || *c == '+') {
            courseIndices.pushBack(i);
        }
    }

    if (courseIndices.empty()) {
        return;
    }

    DEBUG("Adding custom %s pack %s (%u)...", type, path.values(), courseIndices.count());
    packs.pushBack();
    Pack *pack = new (m_heap, 0x4) CustomPack(courseIndices, name, author, version);
    packs.back()->reset(pack);
}

void CourseManager::sortRacePacksByName() {
    SortPacksByName(m_racePacks);
}

void CourseManager::sortBattlePacksByName() {
    SortPacksByName(m_battlePacks);
}

void CourseManager::addDefaultRacePacks() {
    addDefaultPacks(m_raceCourses, m_racePacks, "Courses", "race");
}

void CourseManager::addDefaultBattlePacks() {
    addDefaultPacks(m_battleCourses, m_battlePacks, "Stages", "battle");
}

void CourseManager::addDefaultPacks(const Ring<UniquePtr<Course>, MaxCourseCount> &courses,
        Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *base, const char *type) {
    Array<Ring<u32, MaxCourseCount>, DefaultPackCount> courseIndices;
    Array<Array<char, 32>, DefaultPackCount> names;

    for (u32 i = 0; i < courses.count(); i++) {
        courseIndices[0].pushBack(i);
    }
    snprintf(names[0].values(), names[0].count(), "All%s", base);

    for (u32 i = 0; i < courses.count(); i++) {
        if (courses[i]->isDefault()) {
            courseIndices[1].pushBack(i);
        }
    }
    snprintf(names[1].values(), names[1].count(), "Vanilla%s", base);

    for (u32 i = 0; i < courses.count(); i++) {
        if (courses[i]->isCustom()) {
            courseIndices[2].pushBack(i);
        }
    }
    snprintf(names[2].values(), names[2].count(), "Custom%s", base);

    for (u32 i = DefaultPackCount; i-- > 0;) {
        if (courseIndices[i].empty()) {
            continue;
        }
        DEBUG("Adding default %s pack %s (%u)...", type, names[i].values(),
                courseIndices[i].count());
        if (packs.full()) {
            packs.popBack();
        }
        packs.pushFront();
        Pack *pack = new (m_heap, 0x4) DefaultPack(courseIndices[i], names[i]);
        packs.front()->reset(pack);
    }
}

void CourseManager::sortRacePackCoursesByName() {
    for (u32 i = 0; i < m_racePacks.count(); i++) {
        Ring<u32, MaxCourseCount> &courseIndices = m_racePacks[i]->courseIndices();
        Sort(courseIndices, courseIndices.count(), CompareRaceCourseIndicesByName);
    }
}

void CourseManager::sortBattlePackCoursesByName() {
    for (u32 i = 0; i < m_battlePacks.count(); i++) {
        Ring<u32, MaxCourseCount> &courseIndices = m_battlePacks[i]->courseIndices();
        Sort(courseIndices, courseIndices.count(), CompareBattleCourseIndicesByName);
    }
}

bool CourseManager::hashFile(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const {
    ZIPFile::Reader reader(zipFile, filePath);
    if (!reader.ok()) {
        return false;
    }
    crypto_blake2b_ctx ctx;
    crypto_blake2b_init(&ctx, hash.count());
    for (u32 offset = 0; offset < *reader.size();) {
        const u8 *chunk;
        u32 chunkSize;
        if (!reader.read(chunk, chunkSize)) {
            return false;
        }
        crypto_blake2b_update(&ctx, chunk, chunkSize);
        offset += chunkSize;
    }
    crypto_blake2b_final(&ctx, hash.values());
    return true;
}

bool CourseManager::hashCourseFile(ZIPFile &zipFile, const char *filePath,
        Array<u8, 32> &hash) const {
    if (SZSCourseHasher::Hash(zipFile, filePath, hash)) {
        return true;
    }

    return hashFile(zipFile, filePath, hash);
}

void *CourseManager::loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    ZIPFile zipFile(zipPath);
    if (!zipFile.ok()) {
        return nullptr;
    }
    return loadFile(zipFile, filePath, heap, size);
}

void *CourseManager::loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    ZIPFile::Reader reader(zipFile, filePath);
    if (!reader.ok()) {
        return nullptr;
    }
    UniquePtr<u8[]> data(new (heap, 0x20) u8[*reader.size()]);
    if (!data.get()) {
        return nullptr;
    }
    for (u32 offset = 0; offset < *reader.size();) {
        const u8 *chunk;
        u32 chunkSize;
        if (!reader.read(chunk, chunkSize)) {
            return nullptr;
        }
        memcpy(data.get() + offset, chunk, chunkSize);
        offset += chunkSize;
    }
    if (size) {
        *size = *reader.size();
    }
    return data.release();
}

void *CourseManager::loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
        JKRHeap *heap, u32 *size) const {
    ZIPFile zipFile(zipPath);
    if (!zipFile.ok()) {
        return nullptr;
    }
    return loadLocalizedFile(zipFile, prefix, suffix, heap, size);
}

void *CourseManager::loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix,
        JKRHeap *heap, u32 *size) const {
    for (u32 i = 0; i < languages().count(); i++) {
        const char *languageName = KartLocale::GetLanguageName(languages()[i]);
        Array<char, 256> filePath;
        snprintf(filePath.values(), filePath.count(), "%s%s%s", prefix, languageName, suffix);
        void *data = loadFile(zipFile, filePath.values(), heap, size);
        if (data) {
            return data;
        }
    }
    return nullptr;
}

void *CourseManager::loadLocalizedFile(const char *prefix, const char *suffix, JKRHeap *heap,
        u32 *size) const {
    for (u32 i = 0; i < languages().count(); i++) {
        const char *languageName = KartLocale::GetLanguageName(languages()[i]);
        Array<char, 256> filePath;
        snprintf(filePath.values(), filePath.count(), "%s%s%s", prefix, languageName, suffix);
        void *data = FileLoader::Load(filePath.values(), heap, size);
        if (data) {
            return data;
        }
    }
    return nullptr;
}

void *CourseManager::loadCourseFile(const char *zipPath, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    ZIPFile zipFile(zipPath);
    if (!zipFile.ok()) {
        return nullptr;
    }
    return loadCourseFile(zipFile, filePath, heap, size);
}

void *CourseManager::loadCourseFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    void *data = SZSCourseLoader::Load(zipFile, filePath, heap, size);
    if (data) {
        return data;
    }

    return loadFile(zipFile, filePath, heap, size);
}

bool CourseManager::GetDefaultCourseID(const char *name, u32 &courseID) {
    if (!strcmp(name, "Baby Park")) {
        courseID = CourseID::BabyLuigi;
    } else if (!strcmp(name, "Peach Beach")) {
        courseID = CourseID::Peach;
    } else if (!strcmp(name, "Daisy Cruiser")) {
        courseID = CourseID::Daisy;
    } else if (!strcmp(name, "Luigi Circuit")) {
        courseID = CourseID::Luigi;
    } else if (!strcmp(name, "Mario Circuit")) {
        courseID = CourseID::Mario;
    } else if (!strcmp(name, "Yoshi Circuit")) {
        courseID = CourseID::Yoshi;
    } else if (!strcmp(name, "Mushroom Bridge")) {
        courseID = CourseID::Nokonoko;
    } else if (!strcmp(name, "Mushroom City")) {
        courseID = CourseID::Patapata;
    } else if (!strcmp(name, "Waluigi Stadium")) {
        courseID = CourseID::Waluigi;
    } else if (!strcmp(name, "Wario Colosseum")) {
        courseID = CourseID::Wario;
    } else if (!strcmp(name, "Dino Jungle")) {
        courseID = CourseID::Diddy;
    } else if (!strcmp(name, "Dino Dino Jungle")) {
        courseID = CourseID::Diddy;
    } else if (!strcmp(name, "DK Mountain")) {
        courseID = CourseID::Donkey;
    } else if (!strcmp(name, "Bowser Castle")) {
        courseID = CourseID::Koopa;
    } else if (!strcmp(name, "Rainbow Road")) {
        courseID = CourseID::Rainbow;
    } else if (!strcmp(name, "Dry Dry Desert")) {
        courseID = CourseID::Desert;
    } else if (!strcmp(name, "Sherbet Land")) {
        courseID = CourseID::Snow;
    } else if (!strcmp(name, "Luigi's Mansion")) {
        courseID = CourseID::Mini1;
    } else if (!strcmp(name, "Nintendo Gamecube")) {
        courseID = CourseID::Mini2;
    } else if (!strcmp(name, "Block City")) {
        courseID = CourseID::Mini3;
    } else if (!strcmp(name, "Tilt-a-Kart")) {
        courseID = CourseID::Mini5;
    } else if (!strcmp(name, "Cookie Land")) {
        courseID = CourseID::Mini7;
    } else if (!strcmp(name, "Pipe Plaza")) {
        courseID = CourseID::Mini8;
    } else {
        return false;
    }
    return true;
}

void CourseManager::SortPacksByName(Ring<UniquePtr<Pack>, MaxPackCount> &packs) {
    Sort(packs, packs.count(), ComparePacksByName);
}

bool CourseManager::ComparePacksByName(const UniquePtr<Pack> &a, const UniquePtr<Pack> &b) {
    return strcasecmp(a->name(), b->name()) <= 0;
}

bool CourseManager::CompareRaceCourseIndicesByName(const u32 &a, const u32 &b) {
    Ring<UniquePtr<Course>, MaxCourseCount> &courses = s_instance->m_raceCourses;
    return strcasecmp(courses[a]->name(), courses[b]->name()) <= 0;
}

bool CourseManager::CompareBattleCourseIndicesByName(const u32 &a, const u32 &b) {
    Ring<UniquePtr<Course>, MaxCourseCount> &courses = s_instance->m_battleCourses;
    return strcasecmp(courses[a]->name(), courses[b]->name()) <= 0;
}

CourseManager *CourseManager::s_instance = nullptr;
