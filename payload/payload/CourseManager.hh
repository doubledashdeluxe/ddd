#pragma once

#include "payload/StorageScanner.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <game/MinimapConfig.hh>
#include <jsystem/JKRHeap.hh>

class CourseManager : public StorageScanner {
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

    void process() override;

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
    void *loadCourseFile(const char *zipPath, const char *filePath, u32 *size = nullptr) const;
    void *loadCourseFile(ZIPFile &zipFile, const char *filePath, u32 *size = nullptr) const;

    static int HandlePackINI(void *user, const char *section, const char *name, const char *value);
    static int HandleCourseINI(void *user, const char *section, const char *name,
            const char *value);
    static bool GetDefaultCourseID(const char *name, u32 &courseID);
    static bool SearchJSON(const char *json, u32 jsonSize, const char *query, f32 &value);
    static bool SearchJSON(const char *json, u32 jsonSize, const char *query, u32 &value);
    static void SortPacksByName(Ring<UniquePtr<Pack>, MaxPackCount> &packs);
    static bool ComparePacksByName(const UniquePtr<Pack> &a, const UniquePtr<Pack> &b);
    static bool CompareRaceCourseIndicesByName(const u32 &a, const u32 &b);
    static bool CompareBattleCourseIndicesByName(const u32 &a, const u32 &b);

    JKRHeap *m_heap;
    JKRHeap *m_courseHeap;
    Ring<UniquePtr<Course>, MaxCourseCount> m_raceCourses;
    Ring<UniquePtr<Course>, MaxCourseCount> m_battleCourses;
    Ring<UniquePtr<Pack>, MaxPackCount> m_racePacks;
    Ring<UniquePtr<Pack>, MaxPackCount> m_battlePacks;

    static CourseManager *s_instance;
};
