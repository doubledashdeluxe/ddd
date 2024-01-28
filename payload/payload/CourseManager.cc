#include "CourseManager.hh"

#include "payload/Archive.hh"
#include "payload/FileLoader.hh"
#include "payload/Lock.hh"
#include "payload/SZS.hh"

#include <common/Arena.hh>
#include <common/Log.hh>
extern "C" {
#include <coreJSON/core_json.h>
}
#include <game/CourseID.hh>
#include <game/ResMgr.hh>
extern "C" {
#include <inih/ini.h>
}
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

CourseManager::Course::Course(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 courseID,
        u32 musicID)
    : m_archiveHash(archiveHash), m_bolHash(bolHash), m_courseID(courseID), m_musicID(musicID) {}

CourseManager::Course::~Course() {}

Array<u8, 32> CourseManager::Course::archiveHash() const {
    return m_archiveHash;
}

Array<u8, 32> CourseManager::Course::bolHash() const {
    return m_bolHash;
}

u32 CourseManager::Course::courseID() const {
    return m_courseID;
}

u32 CourseManager::Course::musicID() const {
    return m_musicID;
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

void *CourseManager::DefaultPack::nameImage() const {
    Array<char, 32> name;
    snprintf(name.values(), name.count(), "%s.bti", m_name.values());
    return JKRFileLoader::GetGlbResource(name.values(), nullptr);
}

CourseManager::CustomPack::CustomPack(Ring<u32, MaxCourseCount> courseIndices, char *name,
        char *author, char *version, u8 *nameImage)
    : Pack(courseIndices), m_name(name), m_author(author), m_version(version),
      m_nameImage(nameImage) {}

CourseManager::CustomPack::~CustomPack() {}

const char *CourseManager::CustomPack::name() const {
    return m_name.get();
}

const char *CourseManager::CustomPack::author() const {
    return m_author.get();
}

const char *CourseManager::CustomPack::version() const {
    return m_version.get();
}

void *CourseManager::CustomPack::nameImage() const {
    return m_nameImage.get();
}

CourseManager::DefaultCourse::DefaultCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash,
        u32 courseID, const char *thumbnail, const char *nameImage)
    : Course(archiveHash, bolHash, courseID, courseID), m_thumbnail(thumbnail),
      m_nameImage(nameImage) {}

CourseManager::DefaultCourse::~DefaultCourse() {}

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

void *CourseManager::DefaultCourse::loadLogo() const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    const char *languageName = KartLocale::GetLanguageName();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/CourseName/%s/%s_name.bti", languageName, base);
    return FileLoader::Load(path.values(), s_instance->m_courseHeap);
}

void *CourseManager::DefaultCourse::loadStaffGhost() const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "dvd:/StaffGhosts/%s.ght", base);
    return FileLoader::Load(path.values(), s_instance->m_courseHeap);
}

void *CourseManager::DefaultCourse::loadCourse(u32 courseOrder, u32 raceLevel) const {
    const char *base = ResMgr::GetCrsArcName(m_courseID);
    const char *suffix = courseOrder == 1 ? "L" : "";
    Array<char, 256> path;
    if (m_courseID == 0x24 && raceLevel != 0) {
        snprintf(path.values(), path.count(), "dvd:/Course/Luigi2%s.arc", suffix);
    } else {
        snprintf(path.values(), path.count(), "dvd:/Course/%s%s.arc", base, suffix);
    }
    return FileLoader::Load(path.values(), s_instance->m_courseHeap);
}

bool CourseManager::DefaultCourse::isDefault() const {
    return true;
}

bool CourseManager::DefaultCourse::isCustom() const {
    return false;
}

CourseManager::CustomCourse::CustomCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash,
        u32 courseID, u32 musicID, char *name, char *author, char *version,
        MinimapConfig *minimapConfig, u8 *thumbnail, u8 *nameImage, Array<char, 256> path,
        Array<char, 128> prefix)
    : Course(archiveHash, bolHash, courseID, musicID), m_name(name), m_author(author),
      m_version(version), m_minimapConfig(minimapConfig), m_thumbnail(thumbnail),
      m_nameImage(nameImage), m_path(path), m_prefix(prefix) {}

CourseManager::CustomCourse::~CustomCourse() {}

const char *CourseManager::CustomCourse::name() const {
    return m_name.get();
}

const char *CourseManager::CustomCourse::author() const {
    return m_author.get();
}

const char *CourseManager::CustomCourse::version() const {
    return m_version.get();
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

void *CourseManager::CustomCourse::loadLogo() const {
    Array<char, 256> logoPrefix;
    snprintf(logoPrefix.values(), logoPrefix.count(), "/%scourse_images/", m_prefix.values());
    return s_instance->loadLocalizedFile(m_path.values(), logoPrefix.values(),
            "/track_big_logo.bti", s_instance->m_courseHeap);
}

void *CourseManager::CustomCourse::loadStaffGhost() const {
    Array<char, 256> staffGhostPath;
    snprintf(staffGhostPath.values(), staffGhostPath.count(), "/%sstaffghost.ght",
            m_prefix.values());
    return s_instance->loadFile(m_path.values(), staffGhostPath.values(), s_instance->m_courseHeap);
}

void *CourseManager::CustomCourse::loadCourse(u32 /* courseOrder */, u32 /* raceLevel */) const {
    Array<char, 256> coursePath;
    snprintf(coursePath.values(), coursePath.count(), "/%strack.arc", m_prefix.values());
    return s_instance->loadCourseFile(m_path.values(), coursePath.values());
}

bool CourseManager::CustomCourse::isDefault() const {
    return false;
}

bool CourseManager::CustomCourse::isCustom() const {
    return true;
}

void CourseManager::start() {
    size_t heapSize = 0x500000;
    void *heap = MEM2Arena::Instance()->alloc(heapSize, 0x4);
    m_heap = JKRExpHeap::Create(heap, heapSize, JKRHeap::GetRootHeap(), false);
    size_t courseHeapSize = 0x500000;
    void *courseHeap = MEM2Arena::Instance()->alloc(courseHeapSize, 0x4);
    m_courseHeap = JKRExpHeap::Create(courseHeap, courseHeapSize, JKRHeap::GetRootHeap(), false);
    OSResumeThread(&m_thread);
    OSReceiveMessage(&m_initQueue, nullptr, OS_MESSAGE_BLOCK);
}

bool CourseManager::lock() {
    Lock<NoInterrupts> lock;
    if (m_currIsLocked) {
        return true;
    }
    m_nextIsLocked = true;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    return false;
}

void CourseManager::unlock() {
    Lock<NoInterrupts> lock;
    if (!m_nextIsLocked && !m_currIsLocked) {
        return;
    }
    m_nextIsLocked = false;
    m_currIsLocked = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void CourseManager::freeAll() {
    m_courseHeap->freeAll();
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

CourseManager::CourseManager() : m_currIsLocked(false), m_nextIsLocked(false), m_hasChanged(true) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 12,
            0);
}

void CourseManager::onAdd(const char *prefix) {
    if (strcmp(prefix, "main:")) {
        return;
    }

    Lock<NoInterrupts> lock;
    m_hasChanged = true;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void CourseManager::onRemove(const char *prefix) {
    if (strcmp(prefix, "main:")) {
        return;
    }

    Lock<NoInterrupts> lock;
    m_hasChanged = true;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *CourseManager::run() {
    m_languages = (u32[KartLocale::Language::Max]){
            KartLocale::Language::English,
            KartLocale::Language::French,
            KartLocale::Language::Spanish,
            KartLocale::Language::German,
            KartLocale::Language::Italian,
            KartLocale::Language::Japanese,
    };
    u32 language = KartLocale::GetLanguage();
    u32 i;
    for (i = 0; i < m_languages.count(); i++) {
        if (i == language) {
            break;
        }
    }
    for (; i > 0; i--) {
        m_languages[i] = m_languages[i - 1];
    }
    m_languages[0] = language;

    while (true) {
        {
            Lock<NoInterrupts> lock;
            OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
            m_currIsLocked = m_nextIsLocked;
            if (m_currIsLocked || !m_hasChanged) {
                continue;
            }
            m_hasChanged = false;
            OSSendMessage(&m_initQueue, nullptr, OS_MESSAGE_NOBLOCK);
        }

        m_courseHeap->freeAll();
        m_raceCourses.reset();
        m_battleCourses.reset();
        m_racePacks.reset();
        m_battlePacks.reset();
        addDefaultRaceCourses();
        addDefaultBattleCourses();
        Array<char, 256> path;
        snprintf(path.values(), path.count(), "main:/ddd/courses");
        Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
        Ring<u32, MaxCourseCount> raceCourseIndices;
        Ring<u32, MaxCourseCount> battleCourseIndices;
        addCustomPacksAndCourses(path, raceCourseIndices, battleCourseIndices);
        sortRacePacksByName();
        sortBattlePacksByName();
        addDefaultRacePacks();
        addDefaultBattlePacks();
        sortRacePackCoursesByName();
        sortBattlePackCoursesByName();
    }
}

void CourseManager::addDefaultRaceCourses() {
    Array<Array<u8, 32>, DefaultRaceCourseCount> archiveHashes;
    Array<Array<u8, 32>, DefaultRaceCourseCount> bolHashes;
    Array<u32, DefaultRaceCourseCount> courseIDs;
    Array<const char *, DefaultRaceCourseCount> thumbnails;
    Array<const char *, DefaultRaceCourseCount> nameImages;

    archiveHashes[0] = (u8[32]){0xde, 0x46, 0xd0, 0x58, 0xe3, 0x2a, 0x4e, 0xa7, 0x85, 0xda, 0xd6,
            0xa0, 0x41, 0xf9, 0xca, 0x47, 0x4b, 0x49, 0x65, 0x33, 0x1c, 0xca, 0xb0, 0x3c, 0xbf,
            0x14, 0x28, 0xc9, 0xd5, 0xdd, 0x08, 0xa0};
    bolHashes[0] = (u8[32]){0x59, 0x95, 0x8a, 0x4b, 0x3b, 0x6a, 0x39, 0x17, 0xb5, 0xc8, 0xc4, 0x78,
            0x68, 0x56, 0xed, 0x5d, 0xf3, 0x17, 0x56, 0xd5, 0x17, 0x8c, 0x09, 0x1b, 0x88, 0x10,
            0x55, 0x50, 0x2c, 0x80, 0xe0, 0x73};
    courseIDs[0] = CourseID::BabyLuigi;
    thumbnails[0] = "COP_BABY_PARK.bti";
    nameImages[0] = "CoName_BABY_PARK.bti";

    archiveHashes[1] = (u8[32]){0x45, 0x55, 0x94, 0x11, 0x4e, 0xf3, 0xd8, 0x22, 0xcc, 0x67, 0x5d,
            0x01, 0x02, 0x7f, 0x34, 0xa0, 0x43, 0xd0, 0x8f, 0xda, 0x29, 0x93, 0x4a, 0x75, 0xd9,
            0x5d, 0x3d, 0x68, 0x19, 0x34, 0x19, 0xdb};
    bolHashes[1] = (u8[32]){0xe7, 0xab, 0x55, 0x91, 0xff, 0xab, 0xff, 0x73, 0x7d, 0x6f, 0xe4, 0x93,
            0xd7, 0xa6, 0x4d, 0x09, 0xe3, 0x7a, 0x5e, 0xed, 0x91, 0x6d, 0x40, 0x25, 0x27, 0x55,
            0x8c, 0xa6, 0x48, 0x3f, 0x60, 0xd3};
    courseIDs[1] = CourseID::Peach;
    thumbnails[1] = "COP_PEACH_BEACH.bti";
    nameImages[1] = "CoName_PEACH_BEACH.bti";

    archiveHashes[2] = (u8[32]){0x1d, 0xa8, 0x67, 0x10, 0x53, 0x66, 0x61, 0x79, 0xd4, 0x0d, 0x0a,
            0xcd, 0xa7, 0x29, 0x1d, 0xc0, 0x22, 0xd1, 0xf5, 0xba, 0x5c, 0xe6, 0xf3, 0x1f, 0x8e,
            0x70, 0x43, 0x62, 0xaf, 0x8b, 0x0e, 0x9d};
    bolHashes[2] = (u8[32]){0xbd, 0xd5, 0xca, 0x1e, 0x0f, 0x73, 0xa9, 0x57, 0x84, 0x27, 0xd9, 0xa7,
            0x3e, 0x6c, 0xcf, 0xe5, 0x94, 0xad, 0x29, 0x10, 0xe6, 0xdd, 0xd3, 0xe5, 0x43, 0x38,
            0x4f, 0xeb, 0xf2, 0xc9, 0x8f, 0xab};
    courseIDs[2] = CourseID::Daisy;
    thumbnails[2] = "COP_DAISY_SHIP.bti";
    nameImages[2] = "CoName_DAISY_SHIP.bti";

    archiveHashes[3] = (u8[32]){0xed, 0xc2, 0x0f, 0x19, 0xd1, 0xa0, 0xde, 0xed, 0x3e, 0xac, 0x69,
            0x0b, 0xf9, 0x61, 0x77, 0xe7, 0xce, 0x8a, 0x56, 0xb4, 0xf8, 0x48, 0xaa, 0xcd, 0x63,
            0x12, 0x68, 0xbd, 0x69, 0x12, 0x09, 0x74};
    bolHashes[3] = (u8[32]){0xbd, 0x6b, 0x6a, 0x40, 0x56, 0x23, 0x80, 0x87, 0x75, 0xbc, 0x83, 0x67,
            0xf9, 0x4f, 0xa3, 0x86, 0xdf, 0xb6, 0xe9, 0x42, 0x68, 0x49, 0x92, 0xb6, 0x04, 0xfb,
            0xa1, 0xb2, 0xeb, 0xf2, 0xb1, 0x67};
    courseIDs[3] = CourseID::Luigi;
    thumbnails[3] = "COP_LUIGI_CIRCUIT.bti";
    nameImages[3] = "CoName_LUIGI_CIRCUIT.bti";

    archiveHashes[4] = (u8[32]){0x93, 0xce, 0x7a, 0x45, 0x16, 0x6d, 0xd0, 0x61, 0xcb, 0xb5, 0xb0,
            0x06, 0xca, 0xa0, 0x11, 0xc0, 0x27, 0xa6, 0x47, 0x9e, 0x37, 0x05, 0x35, 0x36, 0x51,
            0x64, 0x4b, 0x7f, 0x8f, 0xa1, 0x63, 0xd3};
    bolHashes[4] = (u8[32]){0x14, 0x33, 0x37, 0xd4, 0x9d, 0x01, 0xe2, 0xc4, 0x38, 0x88, 0x51, 0xd6,
            0x4a, 0x32, 0x78, 0xa0, 0x14, 0x3b, 0xb7, 0xe1, 0x16, 0x95, 0xfe, 0xde, 0x83, 0xde,
            0x92, 0x8f, 0x2b, 0xb1, 0xb2, 0x54};
    courseIDs[4] = CourseID::Mario;
    thumbnails[4] = "COP_MARIO_CIRCUIT.bti";
    nameImages[4] = "CoName_MARIO_CIRCUIT.bti";

    archiveHashes[5] = (u8[32]){0xb6, 0x76, 0xa5, 0xfa, 0xb8, 0x8e, 0xa5, 0x37, 0xe4, 0xb5, 0x2d,
            0x98, 0xe2, 0x80, 0xe8, 0x25, 0xbd, 0x18, 0xd2, 0xf4, 0xa1, 0x76, 0xdb, 0x80, 0xa1,
            0x4a, 0x71, 0x8a, 0x9d, 0xce, 0xce, 0xab};
    bolHashes[5] = (u8[32]){0xee, 0xd7, 0xcc, 0x87, 0xe1, 0x1d, 0xc0, 0x89, 0x11, 0xe3, 0x49, 0xf6,
            0x27, 0x4c, 0xa6, 0x29, 0x25, 0x7d, 0xc2, 0xa4, 0xcd, 0xa6, 0x5d, 0xe5, 0x09, 0xab,
            0xf5, 0x6d, 0x26, 0x85, 0xa3, 0x09};
    courseIDs[5] = CourseID::Yoshi;
    thumbnails[5] = "COP_YOSHI_CIRCUIT.bti";
    nameImages[5] = "CoName_YOSHI_CIRCUIT.bti";

    archiveHashes[6] = (u8[32]){0x49, 0xd2, 0xb6, 0x44, 0x32, 0xf3, 0x83, 0x8a, 0x9c, 0x45, 0x4e,
            0x8d, 0x9f, 0xdc, 0xbf, 0x5b, 0x5f, 0xe2, 0x47, 0x94, 0x97, 0x5f, 0xb6, 0xa7, 0xf2,
            0xe3, 0xa4, 0xfe, 0x5a, 0xc4, 0xe3, 0xbc};
    bolHashes[6] = (u8[32]){0xec, 0xd4, 0xf7, 0x8f, 0xe3, 0x81, 0x2c, 0xd9, 0x85, 0x6e, 0x97, 0x15,
            0x89, 0xd3, 0x9c, 0x2c, 0x6b, 0x3f, 0x25, 0x6b, 0xa2, 0xc4, 0xc0, 0x7e, 0x6a, 0xe8,
            0xe0, 0x73, 0x9d, 0x8d, 0x75, 0x63};
    courseIDs[6] = CourseID::Nokonoko;
    thumbnails[6] = "COP_KINOKO_BRIDGE.bti";
    nameImages[6] = "CoName_KINOKO_BRIDGE.bti";

    archiveHashes[7] = (u8[32]){0x18, 0x7a, 0x5f, 0xf2, 0xca, 0x4e, 0x90, 0xe8, 0xc0, 0x1b, 0x78,
            0x21, 0x39, 0xce, 0xcf, 0x52, 0xbe, 0x55, 0xdf, 0x15, 0x68, 0x48, 0xff, 0x02, 0x72,
            0x5a, 0x48, 0x17, 0xa9, 0xd0, 0xa7, 0xea};
    bolHashes[7] = (u8[32]){0xa2, 0x9e, 0x77, 0x5d, 0x98, 0xfd, 0x3d, 0xd4, 0x39, 0x8d, 0x1f, 0x81,
            0x69, 0x26, 0xd8, 0xb5, 0x81, 0x65, 0x7b, 0xb9, 0x52, 0x74, 0x5e, 0xd7, 0x9d, 0x39,
            0x2e, 0xa4, 0x0a, 0x1e, 0x77, 0xab};
    courseIDs[7] = CourseID::Patapata;
    thumbnails[7] = "COP_KONOKO_CITY.bti";
    nameImages[7] = "CoName_KINOKO_CITY.bti";

    archiveHashes[8] = (u8[32]){0x71, 0x4b, 0xdb, 0x4c, 0x40, 0xaf, 0x00, 0x05, 0x72, 0xb3, 0xa3,
            0x14, 0xf8, 0x33, 0xce, 0x4d, 0x2c, 0xf7, 0x55, 0xf1, 0xfa, 0x8e, 0xe7, 0x58, 0xcc,
            0xb0, 0xb7, 0xcb, 0xc5, 0x75, 0xd3, 0x15};
    bolHashes[8] = (u8[32]){0x94, 0x5e, 0xc8, 0x30, 0xd4, 0xe1, 0x06, 0xce, 0xa1, 0x45, 0xa4, 0xa1,
            0x19, 0x1d, 0xab, 0x0c, 0x04, 0x7f, 0xf4, 0x00, 0xba, 0xf7, 0x3e, 0xb8, 0x24, 0xb0,
            0x43, 0xf7, 0x98, 0x3b, 0x28, 0x98};
    courseIDs[8] = CourseID::Waluigi;
    thumbnails[8] = "COP_WALUIGI_STADIUM.bti";
    nameImages[8] = "CoName_WALUIGI_STADIUM.bti";

    archiveHashes[9] = (u8[32]){0x14, 0x5f, 0xa1, 0x5b, 0xe2, 0xf9, 0xb9, 0x7c, 0x01, 0xe3, 0x7a,
            0x02, 0x72, 0x87, 0x68, 0x29, 0xb5, 0x71, 0x09, 0xf4, 0x36, 0xe3, 0xb2, 0x60, 0x6c,
            0x39, 0x48, 0xee, 0x03, 0xd7, 0x55, 0x10};
    bolHashes[9] = (u8[32]){0x64, 0xef, 0xdd, 0x8c, 0x94, 0x77, 0xcf, 0xee, 0xe5, 0xc0, 0x85, 0x9e,
            0xc5, 0xa0, 0x54, 0xb8, 0x27, 0x8c, 0xbc, 0x18, 0xcc, 0xf1, 0xa2, 0x0f, 0x44, 0x35,
            0x70, 0x32, 0xec, 0x93, 0xdf, 0x75};
    courseIDs[9] = CourseID::Wario;
    thumbnails[9] = "COP_WARIO_COLOSSEUM.bti";
    nameImages[9] = "CoName_WARIO_COLOSSEUM.bti";

    archiveHashes[10] = (u8[32]){0x56, 0x79, 0x52, 0x42, 0xfc, 0x77, 0x75, 0x86, 0xad, 0xa3, 0xd4,
            0x99, 0xf2, 0xae, 0xf2, 0x7e, 0xd8, 0x1e, 0x9c, 0xfa, 0x12, 0x1d, 0xdb, 0xc3, 0xc5,
            0x2d, 0x2e, 0xf5, 0xb0, 0x34, 0x11, 0x1d};
    bolHashes[10] = (u8[32]){0x1a, 0x78, 0x37, 0x74, 0xe9, 0x96, 0xcc, 0x4f, 0xbb, 0x59, 0x11, 0xb3,
            0xd2, 0xe2, 0x69, 0xa8, 0x01, 0xa5, 0xc2, 0x3f, 0xd2, 0xd0, 0x02, 0xe9, 0xcb, 0x2b,
            0xd4, 0x72, 0xdc, 0x88, 0xc1, 0x40};
    courseIDs[10] = CourseID::Diddy;
    thumbnails[10] = "COP_DINO_DINO_JUNGLE.bti";
    nameImages[10] = "CoName_DINO_DINO_JUNGLE.bti";

    archiveHashes[11] = (u8[32]){0x6b, 0xf8, 0x62, 0x29, 0xe9, 0x8d, 0x71, 0xd7, 0x4d, 0xef, 0xde,
            0x1d, 0x3b, 0x17, 0x4b, 0xdc, 0x42, 0x02, 0x28, 0xbc, 0x6e, 0xf0, 0xc8, 0x41, 0xce,
            0x73, 0x29, 0x93, 0x06, 0x66, 0xd5, 0xae};
    bolHashes[11] = (u8[32]){0x0e, 0x50, 0x37, 0x6e, 0x65, 0xec, 0x62, 0x70, 0xc9, 0x82, 0x0b, 0xba,
            0xcc, 0x84, 0xb0, 0xf5, 0x39, 0xa8, 0x60, 0x8e, 0xbe, 0x59, 0x7c, 0x96, 0x62, 0x72,
            0xc2, 0xd7, 0xb7, 0xc3, 0x21, 0x7b};
    courseIDs[11] = CourseID::Donkey;
    thumbnails[11] = "COP_DK_MOUNTAIN.bti";
    nameImages[11] = "CoName_DK_MOUNTAIN.bti";

    archiveHashes[12] = (u8[32]){0xfc, 0xe2, 0x97, 0x0d, 0xa8, 0x10, 0xc5, 0xc5, 0x6a, 0x5f, 0x22,
            0x7d, 0x48, 0x88, 0xba, 0xd6, 0x6e, 0x87, 0xc1, 0x03, 0x90, 0x39, 0x87, 0x33, 0x8f,
            0xf3, 0xf4, 0x6d, 0x2c, 0xbf, 0x13, 0x56};
    bolHashes[12] = (u8[32]){0x49, 0xcd, 0x29, 0xd8, 0x3b, 0xf1, 0xdf, 0xec, 0x84, 0x31, 0x2e, 0x79,
            0xad, 0x24, 0x86, 0x71, 0xf8, 0xdb, 0x83, 0xb9, 0x2c, 0xa9, 0xcc, 0xdb, 0xbc, 0xd4,
            0x3f, 0x4d, 0x2c, 0xf7, 0xe6, 0x1f};
    courseIDs[12] = CourseID::Koopa;
    thumbnails[12] = "COP_BOWSER_CASTLE.bti";
    nameImages[12] = "CoName_BOWSER_CASTLE.bti";

    archiveHashes[13] = (u8[32]){0x48, 0xdf, 0x40, 0x53, 0xf4, 0x6b, 0xaf, 0x42, 0x4f, 0x6d, 0xf6,
            0x7c, 0xa9, 0xe0, 0x18, 0x66, 0xe7, 0x8f, 0x67, 0xda, 0x1f, 0x72, 0xe8, 0x45, 0x60,
            0xcb, 0x43, 0xc9, 0xc1, 0x73, 0x3f, 0xe0};
    bolHashes[13] = (u8[32]){0x49, 0xb8, 0x6b, 0x2d, 0x26, 0xdd, 0xb5, 0x37, 0x0b, 0xa3, 0x38, 0x6d,
            0x35, 0xa8, 0x8e, 0xf2, 0x46, 0x96, 0x84, 0x0f, 0x09, 0x2a, 0xd0, 0xdb, 0x6d, 0x95,
            0x26, 0x92, 0x9a, 0x81, 0x04, 0x95};
    courseIDs[13] = CourseID::Rainbow;
    thumbnails[13] = "COP_RAINBOW_ROAD.bti";
    nameImages[13] = "CoName_RAINBOW_ROAD.bti";

    archiveHashes[14] = (u8[32]){0x72, 0x8e, 0x8f, 0x43, 0x44, 0x3f, 0xdd, 0x1e, 0x1d, 0x2c, 0xd8,
            0x82, 0x96, 0x10, 0x05, 0x42, 0x9b, 0x91, 0x3f, 0x7f, 0x11, 0x2c, 0x3c, 0xd5, 0x57,
            0x78, 0x71, 0x9d, 0x7b, 0x9f, 0xb9, 0xbe};
    bolHashes[14] = (u8[32]){0xbb, 0x9d, 0xc4, 0x00, 0x36, 0xc0, 0xc5, 0x5d, 0xe9, 0x83, 0xfe, 0x01,
            0x5e, 0x25, 0x19, 0xf1, 0x35, 0x43, 0xe7, 0xc0, 0x7b, 0xc2, 0x74, 0xf8, 0x69, 0xab,
            0x6b, 0xd6, 0xcf, 0xe0, 0x0a, 0x6f};
    courseIDs[14] = CourseID::Desert;
    thumbnails[14] = "COP_KARA_KARA_DESERT.bti";
    nameImages[14] = "CoName_KARA_KARA_DESERT.bti";

    archiveHashes[15] = (u8[32]){0xde, 0xa9, 0x65, 0x9a, 0x16, 0x71, 0xa4, 0x9d, 0x0c, 0xb2, 0xe6,
            0x06, 0x66, 0xe0, 0x28, 0x64, 0xae, 0x20, 0x07, 0xe8, 0xf0, 0x3d, 0x23, 0x7a, 0xd7,
            0x4e, 0x7b, 0xe3, 0xdf, 0xa5, 0x8d, 0x5e};
    bolHashes[15] = (u8[32]){0xcc, 0xa9, 0x4e, 0x1e, 0xff, 0x93, 0x40, 0x5f, 0x06, 0x55, 0xac, 0xde,
            0x21, 0xb3, 0x72, 0x04, 0xe9, 0x5c, 0x5d, 0x0c, 0xc7, 0x82, 0x1c, 0x65, 0xbb, 0xa5,
            0x0d, 0x58, 0x7a, 0x36, 0xd9, 0x52};
    courseIDs[15] = CourseID::Snow;
    thumbnails[15] = "COP_SHERBET_LAND.bti";
    nameImages[15] = "CoName_SHERBET_LAND.bti";

    for (u32 i = 0; i < DefaultRaceCourseCount; i++) {
        DEBUG("Adding default race course 0x%02x...\n", courseIDs[i]);
        m_raceCourses.pushBack();
        Course *course = new (m_heap, 0x4) DefaultCourse(archiveHashes[i], bolHashes[i],
                courseIDs[i], thumbnails[i], nameImages[i]);
        m_raceCourses.back()->reset(course);
    }
}

void CourseManager::addDefaultBattleCourses() {
    Array<Array<u8, 32>, DefaultBattleCourseCount> archiveHashes;
    Array<Array<u8, 32>, DefaultBattleCourseCount> bolHashes;
    Array<u32, DefaultBattleCourseCount> courseIDs;
    Array<const char *, DefaultBattleCourseCount> thumbnails;
    Array<const char *, DefaultBattleCourseCount> nameImages;

    archiveHashes[0] = (u8[32]){0x41, 0x20, 0x0c, 0x6c, 0x01, 0xa1, 0xe8, 0x6d, 0xdf, 0xf1, 0xa3,
            0xf0, 0x53, 0xfa, 0xd7, 0x7d, 0x5f, 0x87, 0xbe, 0xa7, 0x3a, 0xdf, 0xe1, 0x0e, 0x48,
            0x39, 0x6d, 0xcc, 0x29, 0x58, 0xff, 0xb6};
    bolHashes[0] = (u8[32]){0x9b, 0x34, 0xea, 0x13, 0x97, 0xe7, 0x5b, 0x88, 0xdb, 0x7f, 0x09, 0xba,
            0xa2, 0xda, 0x03, 0xb3, 0xcd, 0x2b, 0x5b, 0xf9, 0x4a, 0xe1, 0x50, 0xfc, 0xd9, 0x4c,
            0xd3, 0x8c, 0x2d, 0x94, 0xdd, 0xfb};
    courseIDs[0] = CourseID::Mini1;
    thumbnails[0] = "BattleMapSnap6.bti";
    nameImages[0] = "Mozi_Map6.bti";

    archiveHashes[1] = (u8[32]){0xea, 0x3b, 0x9a, 0x47, 0x87, 0x82, 0x8c, 0x1a, 0xf3, 0xbf, 0xcd,
            0x39, 0xdc, 0x51, 0x60, 0xaa, 0x6a, 0x2b, 0x67, 0x6c, 0x62, 0x94, 0x30, 0x33, 0x3f,
            0xee, 0xce, 0xf7, 0xe7, 0xdd, 0x7c, 0x90};
    bolHashes[1] = (u8[32]){0x6f, 0xd8, 0x61, 0x28, 0xa6, 0xa5, 0xbd, 0x04, 0xba, 0x70, 0x00, 0x70,
            0x85, 0x4a, 0x41, 0x05, 0xcc, 0xac, 0x2c, 0x38, 0xb2, 0x2e, 0xef, 0x06, 0xe8, 0x14,
            0xb8, 0xd3, 0xef, 0xce, 0xe3, 0x74};
    courseIDs[1] = CourseID::Mini2;
    thumbnails[1] = "BattleMapSnap3.bti";
    nameImages[1] = "Mozi_Map3.bti";

    archiveHashes[2] = (u8[32]){0x29, 0xab, 0xaf, 0x57, 0xde, 0x86, 0x71, 0x15, 0xab, 0x69, 0xaf,
            0x1b, 0x3d, 0x5b, 0x75, 0x4b, 0x26, 0x0b, 0x84, 0x8d, 0xa1, 0xdf, 0x39, 0x73, 0xd7,
            0x27, 0x2c, 0x7d, 0x55, 0xf1, 0xd6, 0x5d};
    bolHashes[2] = (u8[32]){0x3b, 0x9d, 0x7b, 0x1d, 0xbe, 0x2a, 0x12, 0x0d, 0x02, 0xd0, 0xfa, 0x89,
            0xcf, 0xf9, 0x0e, 0xea, 0x2b, 0x67, 0xf5, 0x7e, 0x25, 0x37, 0xa6, 0x36, 0x69, 0x65,
            0x34, 0xbb, 0x36, 0xec, 0x73, 0x47};
    courseIDs[2] = CourseID::Mini3;
    thumbnails[2] = "BattleMapSnap2.bti";
    nameImages[2] = "Mozi_Map2.bti";

    archiveHashes[3] = (u8[32]){0xac, 0xb4, 0x4e, 0xa2, 0x23, 0x9a, 0xb0, 0x0c, 0xd2, 0xd8, 0x39,
            0x04, 0x07, 0xfe, 0xac, 0x7e, 0x07, 0x09, 0xbd, 0x4f, 0x18, 0xaa, 0x7f, 0x08, 0x5d,
            0x1c, 0x58, 0xb7, 0x8d, 0xa6, 0x98, 0x4b};
    bolHashes[3] = (u8[32]){0x9c, 0x45, 0xd6, 0xb6, 0x3a, 0x4d, 0x54, 0xfd, 0xa4, 0x8e, 0xb9, 0xfb,
            0x0c, 0x61, 0xb2, 0x09, 0x41, 0x0d, 0xd1, 0xbf, 0x51, 0x0a, 0x2d, 0x2f, 0x9f, 0x6c,
            0x1c, 0x66, 0xd0, 0xcf, 0x0d, 0x3f};
    courseIDs[3] = CourseID::Mini5;
    thumbnails[3] = "BattleMapSnap5.bti";
    nameImages[3] = "Mozi_Map5.bti";

    archiveHashes[4] = (u8[32]){0xf8, 0x0d, 0xa9, 0xd7, 0xc2, 0xc8, 0x11, 0x31, 0x51, 0x24, 0x87,
            0x2c, 0x78, 0xd7, 0xd4, 0xd8, 0xb8, 0xc6, 0xf9, 0x2b, 0xcc, 0x5d, 0x89, 0xbb, 0x90,
            0x5d, 0x3b, 0x6d, 0xe9, 0xe9, 0x1f, 0x77};
    bolHashes[4] = (u8[32]){0x3c, 0xec, 0xea, 0x14, 0xa9, 0xbe, 0xb1, 0x61, 0x61, 0x2f, 0x11, 0xa9,
            0x9e, 0x6f, 0x14, 0x96, 0x1a, 0x23, 0x3b, 0xd8, 0x7c, 0x5c, 0x69, 0x68, 0x51, 0x22,
            0xb0, 0xae, 0xd2, 0x46, 0xaa, 0xd9};
    courseIDs[4] = CourseID::Mini7;
    thumbnails[4] = "BattleMapSnap1.bti";
    nameImages[4] = "Mozi_Map1.bti";

    archiveHashes[5] = (u8[32]){0x02, 0x29, 0x79, 0x86, 0x90, 0xa4, 0x94, 0x73, 0x1a, 0x59, 0xdf,
            0x70, 0xa7, 0xda, 0x56, 0x44, 0xa1, 0x30, 0x01, 0xcd, 0x45, 0x5f, 0x47, 0x35, 0x17,
            0x43, 0x92, 0x2c, 0xa3, 0xe2, 0x33, 0x2e};
    bolHashes[5] = (u8[32]){0x7f, 0x45, 0xbb, 0xad, 0x6f, 0x74, 0xbc, 0x68, 0xa0, 0x77, 0x5b, 0x1a,
            0x05, 0x51, 0x3b, 0xc7, 0x32, 0xac, 0x0d, 0xb6, 0x26, 0xc1, 0x0e, 0x7b, 0xf4, 0x94,
            0x47, 0xf7, 0xaa, 0x23, 0x93, 0xfe};
    courseIDs[5] = CourseID::Mini8;
    thumbnails[5] = "BattleMapSnap4.bti";
    nameImages[5] = "Mozi_Map4.bti";

    for (u32 i = 0; i < DefaultBattleCourseCount; i++) {
        DEBUG("Adding default battle course 0x%02x...\n", courseIDs[i]);
        m_battleCourses.pushBack();
        Course *course = new (m_heap, 0x4) DefaultCourse(archiveHashes[i], bolHashes[i],
                courseIDs[i], thumbnails[i], nameImages[i]);
        m_battleCourses.back()->reset(course);
    }
}

void CourseManager::addCustomPacksAndCourses(Array<char, 256> &path,
        Ring<u32, MaxCourseCount> &raceCourseIndices,
        Ring<u32, MaxCourseCount> &battleCourseIndices) {
    u32 length = strlen(path.values());
    Storage::NodeInfo nodeInfo;
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        if (nodeInfo.type == Storage::NodeType::Dir) {
            snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
            addCustomPacksAndCourses(path, raceCourseIndices, battleCourseIndices);
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
    bool ok;
    ZIPFile zipFile(path.values(), ok);
    if (!ok) {
        return;
    }

    ZIPFile::CDNode cdNode;
    ZIPFile::LocalNode localNode;
    INIStream iniStream;
    iniStream.ini.reset(reinterpret_cast<u8 *>(
            zipFile.readFile("trackinfo.ini", m_heap, -0x4, cdNode, localNode)));
    if (!iniStream.ini.get()) {
        return;
    }
    iniStream.iniSize = cdNode.uncompressedSize;
    iniStream.iniOffset = 0x0;

    Array<char, 128> prefix;
    u32 prefixLength = strlen(cdNode.path.get()) - strlen("trackinfo.ini");
    if (prefixLength > prefix.count()) {
        return;
    }
    prefix[prefixLength] = '\0';
    memcpy(prefix.values(), cdNode.path.get(), prefixLength);

    CourseINI courseINI;
    if (ini_parse_stream(ReadINI, &iniStream, HandleCourseINI, &courseINI)) {
        return;
    }

    UniquePtr<char> name(
            getLocalizedEntry(courseINI.localizedNames, courseINI.fallbackName).release());
    if (!name.get()) {
        return;
    }
    UniquePtr<char> author(
            getLocalizedEntry(courseINI.localizedAuthors, courseINI.fallbackAuthor).release());
    UniquePtr<char> version(courseINI.version.release());

    u32 courseID;
    if (!courseINI.defaultCourseName.get() ||
            !GetDefaultCourseID(courseINI.defaultCourseName.get(), courseID)) {
        return;
    }

    u32 musicID;
    if (!courseINI.defaultMusicName.get() ||
            !GetDefaultCourseID(courseINI.defaultMusicName.get(), musicID)) {
        musicID = courseID;
    }

    UniquePtr<MinimapConfig> minimapConfig;
    Array<char, 256> minimapJSONPath;
    snprintf(minimapJSONPath.values(), minimapJSONPath.count(), "/%sminimap.json", prefix.values());
    u32 minimapJSONSize;
    UniquePtr<char[]> minimapJSON(reinterpret_cast<char *>(
            loadFile(zipFile, minimapJSONPath.values(), m_heap, &minimapJSONSize)));
    if (minimapJSON.get()) {
        minimapConfig.reset(readMinimapConfig(minimapJSON.get(), minimapJSONSize));
    }

    Array<char, 256> thumbnailPrefix;
    snprintf(thumbnailPrefix.values(), thumbnailPrefix.count(), "/%scourse_images/",
            prefix.values());
    u32 thumbnailSize;
    UniquePtr<u8> thumbnail(reinterpret_cast<u8 *>(loadLocalizedFile(zipFile,
            thumbnailPrefix.values(), "/track_image.bti", m_heap, &thumbnailSize)));
    if (!thumbnail.get() || thumbnailSize < 0x20) {
        return;
    }

    Array<char, 256> nameImagePrefix;
    snprintf(nameImagePrefix.values(), nameImagePrefix.count(), "/%scourse_images/",
            prefix.values());
    u32 nameImageSize;
    UniquePtr<u8> nameImage(reinterpret_cast<u8 *>(loadLocalizedFile(zipFile,
            nameImagePrefix.values(), "/track_name.bti", m_heap, &nameImageSize)));
    if (!nameImage.get() || nameImageSize < 0x20) {
        return;
    }

    Array<u8, 32> archiveHash;
    Array<u8, 32> bolHash;
    Array<char, 256> hashesPath;
    snprintf(hashesPath.values(), hashesPath.count(), "/%shashes.bin", prefix.values());
    u32 hashesSize;
    UniquePtr<u8> hashes(
            reinterpret_cast<u8 *>(loadFile(zipFile, hashesPath.values(), m_heap, &hashesSize)));
    if (hashes.get() && hashesSize == 64) {
        memcpy(archiveHash.values(), hashes.get() + 0, archiveHash.count());
        memcpy(bolHash.values(), hashes.get() + 32, bolHash.count());
    } else {
        Array<char, 256> coursePath;
        snprintf(coursePath.values(), coursePath.count(), "/%strack.arc", prefix.values());
        u32 courseSize;
        UniquePtr<u8> course(
                reinterpret_cast<u8 *>(loadCourseFile(zipFile, coursePath.values(), &courseSize)));
        if (!course.get()) {
            return;
        }
        crypto_blake2b(archiveHash.values(), archiveHash.count(), course.get(), courseSize);

        Archive archive(course.get());
        if (!archive.isValid(courseSize)) {
            return;
        }
        const char *base = ResMgr::GetCrsArcName(courseID);
        Array<char, 256> bolPath;
        snprintf(bolPath.values(), bolPath.count(), "/%s_course.bol", base);
        const char *name;
        Archive::Dir dir(nullptr);
        Archive::Node node(nullptr);
        bool exists;
        if (!archive.getTree().search(bolPath.values(), name, dir, node, exists)) {
            return;
        }
        if (!exists || !node.isFile()) {
            for (u32 bolCourseID = CourseID::BabyLuigi; bolCourseID <= CourseID::Mini8;
                    bolCourseID++) {
                base = ResMgr::GetCrsArcName(bolCourseID);
                snprintf(bolPath.values(), bolPath.count(), "/%s_course.bol", base);
                if (!archive.getTree().search(bolPath.values(), name, dir, node, exists)) {
                    return;
                }
                if (exists && node.isFile()) {
                    break;
                }
            }
        }
        if (!exists || !node.isFile()) {
            return;
        }
        u8 *bol = reinterpret_cast<u8 *>(node.getFile(archive.getFiles()));
        u32 bolSize = node.getFileSize();
        crypto_blake2b(bolHash.values(), bolHash.count(), bol, bolSize);

        Array<u8, 64> hashes;
        memcpy(hashes.values() + 0, archiveHash.values(), archiveHash.count());
        memcpy(hashes.values() + 32, bolHash.values(), bolHash.count());
        if (!zipFile.writeFile(hashesPath.values(), hashes.values(), hashes.count(), m_heap, 0x4,
                    cdNode, localNode)) {
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
            if (!memcmp(bolHash.values(), (*courses)[i]->bolHash().values(), bolHash.count())) {
                courseIndices->pushBack(i);
                return;
            }
        }
    }

    DEBUG("Adding custom %s course %s...\n", type, path.values());
    courseIndices->pushBack(courses->count());
    courses->pushBack();
    Course *course = new (m_heap, 0x4) CustomCourse(archiveHash, bolHash, courseID, musicID,
            name.release(), author.release(), version.release(), minimapConfig.release(),
            thumbnail.release(), nameImage.release(), path, prefix);
    courses->back()->reset(course);
}

MinimapConfig *CourseManager::readMinimapConfig(const char *json, u32 jsonSize) {
    if (JSON_Validate(json, jsonSize) != JSONSuccess) {
        return nullptr;
    }

    UniquePtr<MinimapConfig> minimapConfig(new (m_heap, 0x4) MinimapConfig);
    if (!SearchJSON(json, jsonSize, "Top Left Corner X", minimapConfig->box.start.x)) {
        return nullptr;
    }
    if (!SearchJSON(json, jsonSize, "Top Left Corner Z", minimapConfig->box.start.y)) {
        return nullptr;
    }
    if (!SearchJSON(json, jsonSize, "Bottom Right Corner X", minimapConfig->box.end.x)) {
        return nullptr;
    }
    if (!SearchJSON(json, jsonSize, "Bottom Right Corner Z", minimapConfig->box.end.y)) {
        return nullptr;
    }
    if (!SearchJSON(json, jsonSize, "Orientation", minimapConfig->orientation)) {
        return nullptr;
    }

    return minimapConfig.release();
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
    INIStream iniStream;
    iniStream.ini.reset(
            reinterpret_cast<u8 *>(FileLoader::Load(iniPath.values(), m_heap, &iniStream.iniSize)));
    if (!iniStream.ini.get()) {
        return;
    }
    iniStream.iniOffset = 0x0;

    PackINI packINI;
    if (ini_parse_stream(ReadINI, &iniStream, HandlePackINI, &packINI)) {
        return;
    }

    UniquePtr<char> name(getLocalizedEntry(packINI.localizedNames, packINI.fallbackName).release());
    if (!name.get()) {
        return;
    }
    UniquePtr<char> author(
            getLocalizedEntry(packINI.localizedAuthors, packINI.fallbackAuthor).release());
    UniquePtr<char> version(packINI.version.release());

    Array<char, 256> nameImagePrefix;
    snprintf(nameImagePrefix.values(), nameImagePrefix.count(), "%s/pack_images/", path.values());
    u32 nameImageSize;
    UniquePtr<u8> nameImage(reinterpret_cast<u8 *>(
            loadLocalizedFile(nameImagePrefix.values(), "/pack_name.bti", m_heap, &nameImageSize)));
    if (!nameImage.get() || nameImageSize < 0x20) {
        return;
    }

    if (packINI.defaultCourses) {
        const char *c = packINI.defaultCourses.get();
        for (u32 i = 0; i < defaultCourseOffset && *c; i++, c++) {}
        for (u32 i = 0; i < defaultCourseCount && *c; i++, c++) {
            if (*c == 'Y' || *c == 'y' || *c == '+') {
                courseIndices.pushBack(i);
            }
        }
    }

    if (courseIndices.empty()) {
        return;
    }

    DEBUG("Adding custom %s pack %s (%u)...\n", type, path.values(), courseIndices.count());
    packs.pushBack();
    Pack *pack = new (m_heap, 0x4) CustomPack(courseIndices, name.release(), author.release(),
            version.release(), nameImage.release());
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
        DEBUG("Adding default %s pack %s (%u)...\n", type, names[i].values(),
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

UniquePtr<char> &CourseManager::getLocalizedEntry(
        Array<UniquePtr<char>, KartLocale::Language::Max> &localizedEntries,
        UniquePtr<char> &fallbackEntry) {
    for (u32 i = 0; i < KartLocale::Language::Max; i++) {
        if (localizedEntries[m_languages[i]].get()) {
            return localizedEntries[m_languages[i]];
        }
    }
    return fallbackEntry;
}

void *CourseManager::loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    bool ok;
    ZIPFile zipFile(zipPath, ok);
    if (!ok) {
        return nullptr;
    }
    return loadFile(zipFile, filePath, heap, size);
}

void *CourseManager::loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    ZIPFile::CDNode cdNode;
    ZIPFile::LocalNode localNode;
    void *file = zipFile.readFile(filePath, heap, 0x20, cdNode, localNode);
    if (!file) {
        return nullptr;
    }
    if (size) {
        *size = cdNode.uncompressedSize;
    }
    return file;
}

void *CourseManager::loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
        JKRHeap *heap, u32 *size) const {
    bool ok;
    ZIPFile zipFile(zipPath, ok);
    if (!ok) {
        return nullptr;
    }
    return loadLocalizedFile(zipFile, prefix, suffix, heap, size);
}

void *CourseManager::loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix,
        JKRHeap *heap, u32 *size) const {
    for (u32 i = 0; i < m_languages.count(); i++) {
        const char *languageName = KartLocale::GetLanguageName(m_languages[i]);
        Array<char, 256> filePath;
        snprintf(filePath.values(), filePath.count(), "%s%s%s", prefix, languageName, suffix);
        void *file = loadFile(zipFile, filePath.values(), heap, size);
        if (file) {
            return file;
        }
    }
    return nullptr;
}

void *CourseManager::loadLocalizedFile(const char *prefix, const char *suffix, JKRHeap *heap,
        u32 *size) const {
    for (u32 i = 0; i < m_languages.count(); i++) {
        const char *languageName = KartLocale::GetLanguageName(m_languages[i]);
        Array<char, 256> filePath;
        snprintf(filePath.values(), filePath.count(), "%s%s%s", prefix, languageName, suffix);
        void *file = FileLoader::Load(filePath.values(), heap, size);
        if (file) {
            return file;
        }
    }
    return nullptr;
}

void *CourseManager::loadCourseFile(const char *zipPath, const char *filePath, u32 *size) const {
    bool ok;
    ZIPFile zipFile(zipPath, ok);
    if (!ok) {
        return nullptr;
    }
    return loadCourseFile(zipFile, filePath, size);
}

void *CourseManager::loadCourseFile(ZIPFile &zipFile, const char *filePath, u32 *size) const {
    u32 compressedSize;
    UniquePtr<u8> compressed(
            reinterpret_cast<u8 *>(loadFile(zipFile, filePath, m_courseHeap, &compressedSize)));
    if (!compressed.get()) {
        return nullptr;
    }
    UniquePtr<u8> uncompressed;
    u32 uncompressedSize;
    if (SZS::GetUncompressedSize(compressed.get(), compressedSize, uncompressedSize)) {
        uncompressed.reset(new (m_courseHeap, 0x20) u8[uncompressedSize]);
        if (!SZS::Uncompress(compressed.get(), compressedSize, uncompressed.get(),
                    uncompressedSize)) {
            return nullptr;
        }
    } else {
        uncompressed.reset(compressed.release());
        uncompressedSize = compressedSize;
    }
    if (size) {
        *size = uncompressedSize;
    }
    return uncompressed.release();
}

void *CourseManager::Run(void *param) {
    return reinterpret_cast<CourseManager *>(param)->run();
}

char *CourseManager::ReadINI(char *str, int num, void *stream) {
    INIStream *iniStream = reinterpret_cast<INIStream *>(stream);
    if (iniStream->iniOffset == iniStream->iniSize || num < 2) {
        return nullptr;
    }

    char *s;
    for (s = str; num > 1 && iniStream->iniOffset < iniStream->iniSize; num--) {
        char c = iniStream->ini.get()[iniStream->iniOffset++];
        *s++ = c;
        if (c == '\n') {
            break;
        }
    }
    *s = '\0';
    return str;
}

int CourseManager::HandlePackINI(void *user, const char *section, const char *name,
        const char *value) {
    if (strcmp(section, "Config")) {
        return 1;
    }

    PackINI *packINI = reinterpret_cast<PackINI *>(user);
    Array<INIField, 4> iniFields;
    iniFields[0] = (INIField){"packname", &packINI->fallbackName};
    iniFields[1] = (INIField){"author", &packINI->fallbackAuthor};
    iniFields[2] = (INIField){"version", &packINI->version};
    iniFields[3] = (INIField){"vanilla_tracks", &packINI->defaultCourses};
    if (HandleINIFields(name, value, iniFields.count(), iniFields.values())) {
        return 1;
    }

    Array<LocalizedINIField, 2> localizedINIFields;
    localizedINIFields[0] = (LocalizedINIField){"packname_", &packINI->localizedNames};
    localizedINIFields[1] = (LocalizedINIField){"author_", &packINI->localizedAuthors};
    if (HandleLocalizedINIFields(name, value, localizedINIFields.count(),
                localizedINIFields.values())) {
        return 1;
    }

    return 1;
}

int CourseManager::HandleCourseINI(void *user, const char *section, const char *name,
        const char *value) {
    if (strcmp(section, "Config")) {
        return 1;
    }

    CourseINI *courseINI = reinterpret_cast<CourseINI *>(user);
    Array<INIField, 5> iniFields;
    iniFields[0] = (INIField){"trackname", &courseINI->fallbackName};
    iniFields[1] = (INIField){"author", &courseINI->fallbackAuthor};
    iniFields[2] = (INIField){"version", &courseINI->version};
    iniFields[3] = (INIField){"replaces", &courseINI->defaultCourseName};
    iniFields[4] = (INIField){"auxiliary_audio_track", &courseINI->defaultMusicName};
    if (HandleINIFields(name, value, iniFields.count(), iniFields.values())) {
        return 1;
    }

    Array<LocalizedINIField, 2> localizedINIFields;
    localizedINIFields[0] = (LocalizedINIField){"trackname_", &courseINI->localizedNames};
    localizedINIFields[1] = (LocalizedINIField){"author_", &courseINI->localizedAuthors};
    if (HandleLocalizedINIFields(name, value, localizedINIFields.count(),
                localizedINIFields.values())) {
        return 1;
    }

    return 1;
}

bool CourseManager::HandleINIFields(const char *name, const char *value, u32 fieldCount,
        const INIField *fields) {
    for (u32 i = 0; i < fieldCount; i++) {
        if (strcasecmp(name, fields[i].name)) {
            continue;
        }
        if (SetINIField(value, fields[i].field)) {
            return true;
        }
    }
    return false;
}

bool CourseManager::HandleLocalizedINIFields(const char *name, const char *value, u32 fieldCount,
        const LocalizedINIField *fields) {
    for (u32 i = 0; i < fieldCount; i++) {
        if (strncasecmp(name, fields[i].name, strlen(fields[i].name))) {
            continue;
        }
        for (u32 j = 0; j < KartLocale::Language::Max; j++) {
            if (strcasecmp(name + strlen(fields[i].name), KartLocale::GetLanguageName(j))) {
                continue;
            }
            if (SetINIField(value, &(*fields[i].fields)[j])) {
                return true;
            }
        }
    }
    return false;
}

bool CourseManager::SetINIField(const char *value, UniquePtr<char> *field) {
    u32 valueLength = strlen(value);
    field->reset(new (s_instance->m_heap, 0x4) char[valueLength + 1]);
    if (!field->get()) {
        return false;
    }
    field->get()[valueLength] = '\0';
    memcpy(field->get(), value, valueLength);
    return true;
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

bool CourseManager::SearchJSON(const char *json, u32 jsonSize, const char *query, f32 &value) {
    const char *svalue;
    size_t svalueSize;
    JSONTypes_t type;
    if (JSON_SearchConst(json, jsonSize, query, strlen(query), &svalue, &svalueSize, &type) !=
            JSONSuccess) {
        return false;
    }
    if (type != JSONNumber) {
        return false;
    }

    return sscanf(svalue, "%f", &value) == 1;
}

bool CourseManager::SearchJSON(const char *json, u32 jsonSize, const char *query, u32 &value) {
    const char *svalue;
    size_t svalueSize;
    JSONTypes_t type;
    if (JSON_SearchConst(json, jsonSize, query, strlen(query), &svalue, &svalueSize, &type) !=
            JSONSuccess) {
        return false;
    }
    if (type != JSONNumber) {
        return false;
    }

    return sscanf(svalue, "%u", &value) == 1;
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
