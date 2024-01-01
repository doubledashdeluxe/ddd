#pragma once

#include "payload/FileArchive.hh"
#include "payload/ZIPFile.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <game/KartLocale.hh>
#include <jsystem/JKRHeap.hh>

class CourseManager : public Storage::Observer {
public:
    enum {
        MaxCourseCount = 256,
        MaxPackCount = 128,
    };

public:
    class Pack {
    public:
        Pack(Ring<u32, MaxCourseCount> courseIndices);
        virtual ~Pack();

        u32 courseCount() const;
        u32 courseIndex(u32 index) const;

        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
        virtual void *nameImage() const = 0;

    protected:
        Ring<u32, MaxCourseCount> m_courseIndices;
    };

    class Course {
    public:
        Course(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 courseID, u32 musicID);
        virtual ~Course();

        Array<u8, 32> archiveHash() const;
        Array<u8, 32> bolHash() const;
        u32 courseID() const;
        u32 musicID() const;

        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
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
        u32 m_courseID;
        u32 m_musicID;
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
        UniquePtr<char> m_name;
        UniquePtr<char> m_author;
        UniquePtr<char> m_version;
        UniquePtr<u8> m_nameImage;
    };

    class DefaultCourse : public Course {
    public:
        DefaultCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 courseID,
                const char *thumbnail, const char *nameImage);
        ~DefaultCourse() override;

        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        void *thumbnail() const override;
        void *nameImage() const override;
        void *loadLogo() const override;
        void *loadStaffGhost() const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        const char *m_thumbnail;
        const char *m_nameImage;
    };

    class CustomCourse : public Course {
    public:
        CustomCourse(Array<u8, 32> archiveHash, Array<u8, 32> bolHash, u32 courseID, u32 musicID,
                char *name, char *author, char *version, u8 *thumbnail, u8 *nameImage,
                Array<char, 256> path, Array<char, 128> prefix);
        ~CustomCourse() override;

        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        void *thumbnail() const override;
        void *nameImage() const override;
        void *loadLogo() const override;
        void *loadStaffGhost() const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        UniquePtr<char> m_name;
        UniquePtr<char> m_author;
        UniquePtr<char> m_version;
        UniquePtr<u8> m_thumbnail;
        UniquePtr<u8> m_nameImage;
        Array<char, 256> m_path;
        Array<char, 128> m_prefix;
    };

    struct INIStream {
        UniquePtr<u8> ini;
        u32 iniSize;
        u32 iniOffset;
    };

    struct CourseINI {
        Array<UniquePtr<char>, KartLocale::Language::Max> localizedNames;
        UniquePtr<char> fallbackName;
        Array<UniquePtr<char>, KartLocale::Language::Max> localizedAuthors;
        UniquePtr<char> fallbackAuthor;
        UniquePtr<char> version;
        UniquePtr<char> defaultCourseName;
        UniquePtr<char> defaultMusicName;
    };

    CourseManager();

    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    void *run();
    void addDefaultRaceCourses();
    void addDefaultBattleCourses();
    void addCustomCourses(Array<char, 256> &path);
    void addCustomCourse(const Array<char, 256> &path);
    void sortRaceCoursesByName();
    void sortBattleCoursesByName();
    void deduplicateRaceCourses();
    void deduplicateBattleCourses();
    void addCustomPacks(Array<char, 256> &path);
    void addCustomRacePack(const Array<char, 256> &path);
    void addCustomBattlePack(const Array<char, 256> &path);
    void addCustomPack(const Array<char, 256> &path,
            const Ring<UniquePtr<Course>, MaxCourseCount> &courses,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *type);
    void sortRacePacksByName();
    void sortBattlePacksByName();
    void addDefaultRacePacks();
    void addDefaultBattlePacks();
    void addDefaultPacks(const Ring<UniquePtr<Course>, MaxCourseCount> &courses,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *base, const char *type);
    UniquePtr<char> &getLocalizedEntry(
            Array<UniquePtr<char>, KartLocale::Language::Max> &localizedEntries,
            UniquePtr<char> &fallbackEntry);
    bool loadSubfile(FileArchive &archive, const char *filePath, void *subfile, u32 size) const;
    void *loadSubfile(const char *archivePath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadSubfile(FileArchive &archive, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    bool loadLocalizedSubfile(FileArchive &archive, const char *filePathPattern, void *subfile,
            u32 size) const;
    void *loadLocalizedSubfile(const char *archivePath, const char *filePathPattern, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedSubfile(FileArchive &archive, const char *filePathPattern, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
            JKRHeap *heap, u32 *size = nullptr) const;
    void *loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix, JKRHeap *heap,
            u32 *size = nullptr) const;

    static void *Run(void *param);
    static char *ReadINI(char *str, int num, void *stream);
    static int HandleCourseINI(void *user, const char *section, const char *name,
            const char *value);
    static bool GetDefaultCourseID(const char *name, u32 &courseID);
    static void SortCoursesByName(Ring<UniquePtr<Course>, MaxCourseCount> &courses);
    static void DeduplicateCourses(Ring<UniquePtr<Course>, MaxCourseCount> &courses);
    static void SortPacksByName(Ring<UniquePtr<Pack>, MaxPackCount> &packs);
    static bool CompareCoursesByHash(const UniquePtr<Course> &a, const UniquePtr<Course> &b);
    static bool CompareCoursesByName(const UniquePtr<Course> &a, const UniquePtr<Course> &b);
    static bool ComparePacksByName(const UniquePtr<Pack> &a, const UniquePtr<Pack> &b);

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;
    Array<u8, 16 * 1024> m_stack;
    OSThread m_thread;
    bool m_currIsLocked;
    bool m_nextIsLocked;
    bool m_hasChanged;
    JKRHeap *m_heap;
    JKRHeap *m_courseHeap;
    Array<u32, KartLocale::Language::Max> m_languages;
    Ring<UniquePtr<Course>, MaxCourseCount> m_raceCourses;
    Ring<UniquePtr<Course>, MaxCourseCount> m_battleCourses;
    Ring<UniquePtr<Pack>, MaxPackCount> m_racePacks;
    Ring<UniquePtr<Pack>, MaxPackCount> m_battlePacks;

    static CourseManager *s_instance;
};
