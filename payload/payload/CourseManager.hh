#pragma once

#include "payload/ZIPFile.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <game/KartLocale.hh>
#include <game/MinimapConfig.hh>
#include <jsystem/JKRHeap.hh>

class CourseManager : public Storage::Observer {
public:
    enum {
        MaxCourseCount = 256,
        MaxPackCount = 128,
    };

    class Pack {
    public:
        Pack(Ring<u32, MaxCourseCount> courseIndices);
        virtual ~Pack();

        const Ring<u32, MaxCourseCount> &courseIndices() const;
        Ring<u32, MaxCourseCount> &courseIndices();

        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
        virtual void *nameImage() const = 0;

    protected:
        Ring<u32, MaxCourseCount> m_courseIndices;
    };

    class Course {
    public:
        Course(Array<u8, 32> archiveHash, Array<u8, 32> bolHash);
        virtual ~Course();

        Array<u8, 32> archiveHash() const;
        Array<u8, 32> bolHash() const;

        virtual u32 musicID() const = 0;
        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
        virtual const MinimapConfig *minimapConfig() const = 0;
        virtual void *thumbnail() const = 0;
        virtual void *nameImage() const = 0;
        virtual void *loadLogo() const = 0;
        virtual void *loadStaffGhost() const = 0;
        virtual void *loadCourse(u32 courseOrder, u32 raceLevel) const = 0;
        virtual bool isDefault() const = 0;
        virtual bool isCustom() const = 0;

    protected:
        Array<u8, 32> m_archiveHash;
        Array<u8, 32> m_bolHash;
    };

    void start();
    bool lock();
    void unlock();
    void freeAll();

    u32 racePackCount() const;
    u32 battlePackCount() const;
    const Pack &racePack(u32 index) const;
    const Pack &battlePack(u32 index) const;
    u32 raceCourseCount(u32 packIndex) const;
    u32 battleCourseCount(u32 packIndex) const;
    const Course &raceCourse(u32 packIndex, u32 index) const;
    const Course &battleCourse(u32 packIndex, u32 index) const;

    static void Init();
    static CourseManager *Instance();

private:
    enum {
        DefaultRaceCourseCount = 16,
        DefaultBattleCourseCount = 6,
        DefaultPackCount = 3,
    };

    class DefaultPack : public Pack {
    public:
        DefaultPack(Ring<u32, MaxCourseCount> courseIndices, Array<char, 32> name);
        ~DefaultPack() override;

        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        void *nameImage() const override;

    private:
        Array<char, 32> m_name;
    };

    class CustomPack : public Pack {
    public:
        CustomPack(Ring<u32, MaxCourseCount> courseIndices, char *name, char *author, char *version,
                u8 *nameImage);
        ~CustomPack();
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        void *nameImage() const override;

    private:
        UniquePtr<char[]> m_name;
        UniquePtr<char[]> m_author;
        UniquePtr<char[]> m_version;
        UniquePtr<u8[]> m_nameImage;
    };

    class DefaultCourse : public Course {
    public:
        DefaultCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 courseID,
                const char *thumbnail, const char *nameImage);
        ~DefaultCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *thumbnail() const override;
        void *nameImage() const override;
        void *loadLogo() const override;
        void *loadStaffGhost() const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_courseID;
        const char *m_thumbnail;
        const char *m_nameImage;
    };

    class CustomCourse : public Course {
    public:
        CustomCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 musicID, char *name,
                char *author, char *version, MinimapConfig *minimapConfig, u8 *thumbnail,
                u8 *nameImage, Array<char, 256> path, Array<char, 128> prefix);
        ~CustomCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *thumbnail() const override;
        void *nameImage() const override;
        void *loadLogo() const override;
        void *loadStaffGhost() const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_musicID;
        UniquePtr<char[]> m_name;
        UniquePtr<char[]> m_author;
        UniquePtr<char[]> m_version;
        UniquePtr<MinimapConfig> m_minimapConfig;
        UniquePtr<u8[]> m_thumbnail;
        UniquePtr<u8[]> m_nameImage;
        Array<char, 256> m_path;
        Array<char, 128> m_prefix;
    };

    struct INIStream {
        UniquePtr<u8[]> ini;
        u32 iniSize;
        u32 iniOffset;
    };

    struct INIField {
        const char *name;
        UniquePtr<char[]> *field;
    };

    struct LocalizedINIField {
        const char *name;
        Array<UniquePtr<char[]>, KartLocale::Language::Count> *fields;
    };

    struct PackINI {
        Array<UniquePtr<char[]>, KartLocale::Language::Count> localizedNames;
        UniquePtr<char[]> fallbackName;
        Array<UniquePtr<char[]>, KartLocale::Language::Count> localizedAuthors;
        UniquePtr<char[]> fallbackAuthor;
        UniquePtr<char[]> version;
        UniquePtr<char[]> defaultCourses;
    };

    struct CourseINI {
        Array<UniquePtr<char[]>, KartLocale::Language::Count> localizedNames;
        UniquePtr<char[]> fallbackName;
        Array<UniquePtr<char[]>, KartLocale::Language::Count> localizedAuthors;
        UniquePtr<char[]> fallbackAuthor;
        UniquePtr<char[]> version;
        UniquePtr<char[]> defaultCourseName;
        UniquePtr<char[]> defaultMusicName;
    };

    CourseManager();

    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    void *run();
    void addDefaultRaceCourses();
    void addDefaultBattleCourses();
    void addCustomPacksAndCourses(Array<char, 256> &path,
            Ring<u32, MaxCourseCount> &raceCourseIndices,
            Ring<u32, MaxCourseCount> &battleCourseIndices);
    void addCustomCourse(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &raceCourseIndices,
            Ring<u32, MaxCourseCount> &battleCourseIndices);
    MinimapConfig *readMinimapConfig(const char *json, u32 jsonSize);
    void addCustomRacePack(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &courseIndices);
    void addCustomBattlePack(const Array<char, 256> &path,
            Ring<u32, MaxCourseCount> &courseIndices);
    void addCustomPack(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &courseIndices,
            u32 defaultCourseOffset, u32 defaultCourseCount,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *type);
    void sortRacePacksByName();
    void sortBattlePacksByName();
    void addDefaultRacePacks();
    void addDefaultBattlePacks();
    void addDefaultPacks(const Ring<UniquePtr<Course>, MaxCourseCount> &courses,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *base, const char *type);
    void sortRacePackCoursesByName();
    void sortBattlePackCoursesByName();
    UniquePtr<char[]> &getLocalizedEntry(
            Array<UniquePtr<char[]>, KartLocale::Language::Count> &localizedEntries,
            UniquePtr<char[]> &fallbackEntry);
    void *loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
            JKRHeap *heap, u32 *size = nullptr) const;
    void *loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedFile(const char *prefix, const char *suffix, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadCourseFile(const char *zipPath, const char *filePath, u32 *size = nullptr) const;
    void *loadCourseFile(ZIPFile &zipFile, const char *filePath, u32 *size = nullptr) const;

    static void *Run(void *param);
    static char *ReadINI(char *str, int num, void *stream);
    static int HandlePackINI(void *user, const char *section, const char *name, const char *value);
    static int HandleCourseINI(void *user, const char *section, const char *name,
            const char *value);
    static bool HandleINIFields(const char *name, const char *value, u32 fieldCount,
            const INIField *fields);
    static bool HandleLocalizedINIFields(const char *name, const char *value, u32 fieldCount,
            const LocalizedINIField *fields);
    static bool SetINIField(const char *value, UniquePtr<char[]> *field);
    static bool GetDefaultCourseID(const char *name, u32 &courseID);
    static bool SearchJSON(const char *json, u32 jsonSize, const char *query, f32 &value);
    static bool SearchJSON(const char *json, u32 jsonSize, const char *query, u32 &value);
    static void SortPacksByName(Ring<UniquePtr<Pack>, MaxPackCount> &packs);
    static bool ComparePacksByName(const UniquePtr<Pack> &a, const UniquePtr<Pack> &b);
    static bool CompareRaceCourseIndicesByName(const u32 &a, const u32 &b);
    static bool CompareBattleCourseIndicesByName(const u32 &a, const u32 &b);

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;
    Array<u8, 128 * 1024> m_stack;
    OSThread m_thread;
    bool m_currIsLocked;
    bool m_nextIsLocked;
    bool m_hasChanged;
    JKRHeap *m_heap;
    JKRHeap *m_courseHeap;
    Array<u32, KartLocale::Language::Count> m_languages;
    Ring<UniquePtr<Course>, MaxCourseCount> m_raceCourses;
    Ring<UniquePtr<Course>, MaxCourseCount> m_battleCourses;
    Ring<UniquePtr<Pack>, MaxPackCount> m_racePacks;
    Ring<UniquePtr<Pack>, MaxPackCount> m_battlePacks;

    static CourseManager *s_instance;
};
