#pragma once

#include "payload/StorageScanner.hh"
#include "payload/ZIPFile.hh"

#include <common/Array.hh>
#include <common/Optional.hh>
#include <common/Ring.hh>
#include <common/storage/Storage.hh>
#include <game/MinimapConfig.hh>
#include <jsystem/JKRHeap.hh>

class CourseManager : public StorageScanner {
public:
    enum {
        MaxCourseCount = 256,
        MaxPackCount = 64,
    };

    class Pack {
    public:
        Pack(Ring<u8, MaxCourseCount> courseIndices);
        virtual ~Pack();

        const Ring<u8, MaxCourseCount> &courseIndices() const;
        Ring<u8, MaxCourseCount> &courseIndices();

        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;

    protected:
        Ring<u8, MaxCourseCount> m_courseIndices;
    };

    class Course {
    public:
        Course(Array<u8, 32> archiveHash);
        virtual ~Course();

        Array<u8, 32> archiveHash() const;

        virtual u32 musicID() const = 0;
        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
        virtual const MinimapConfig *minimapConfig() const = 0;
        virtual void *loadThumbnail(JKRHeap *heap) const = 0;
        virtual void *loadNameImage(JKRHeap *heap) const = 0;
        virtual void *loadLogo(JKRHeap *heap) const = 0;
        virtual void *loadStaffGhost(JKRHeap *heap) const = 0;
        virtual void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap,
                u32 &courseSize) const = 0;
        virtual bool isDefault() const = 0;
        virtual bool isCustom() const = 0;

    protected:
        Array<u8, 32> m_archiveHash;
    };

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
        MaxCustomRaceCourseCount = MaxCourseCount - DefaultRaceCourseCount,
        MaxCustomBattleCourseCount = MaxCourseCount - DefaultBattleCourseCount,
        DefaultPackCount = 3,
        MaxCustomPackCount = MaxPackCount - DefaultPackCount,
    };

    class DefaultPack : public Pack {
    public:
        DefaultPack(Ring<u8, MaxCourseCount> courseIndices, Array<char, 32> name);
        ~DefaultPack() override;

        const char *name() const override;
        const char *author() const override;
        const char *version() const override;

    private:
        Array<char, 32> m_name;
    };

    class CustomPack : public Pack {
    public:
        CustomPack(Ring<u8, MaxCourseCount> courseIndices, Array<char, INIReader::FieldSize> name,
                Array<char, INIReader::FieldSize> author,
                Array<char, INIReader::FieldSize> version);
        ~CustomPack();
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;

    private:
        Array<char, INIReader::FieldSize> m_name;
        Array<char, INIReader::FieldSize> m_author;
        Array<char, INIReader::FieldSize> m_version;
    };

    class DefaultCourse : public Course {
    public:
        DefaultCourse(Array<u8, 32> archiveHash, u32 courseID, const char *thumbnail,
                const char *nameImage);
        ~DefaultCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *loadThumbnail(JKRHeap *heap) const override;
        void *loadNameImage(JKRHeap *heap) const override;
        void *loadLogo(JKRHeap *heap) const override;
        void *loadStaffGhost(JKRHeap *heap) const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap,
                u32 &courseSize) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_courseID;
        const char *m_thumbnail;
        const char *m_nameImage;
    };

    class CustomCourse : public Course {
    public:
        CustomCourse(Array<u8, 32> archiveHash, u32 musicID, Array<char, INIReader::FieldSize> name,
                Array<char, INIReader::FieldSize> author, Array<char, INIReader::FieldSize> version,
                Optional<MinimapConfig> minimapConfig, Array<char, 256> path,
                Array<char, 128> prefix);
        ~CustomCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *loadThumbnail(JKRHeap *heap) const override;
        void *loadNameImage(JKRHeap *heap) const override;
        void *loadLogo(JKRHeap *heap) const override;
        void *loadStaffGhost(JKRHeap *heap) const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap,
                u32 &courseSize) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_musicID;
        Array<char, INIReader::FieldSize> m_name;
        Array<char, INIReader::FieldSize> m_author;
        Array<char, INIReader::FieldSize> m_version;
        Optional<MinimapConfig> m_minimapConfig;
        Array<char, 256> m_path;
        Array<char, 128> m_prefix;
    };

    struct PackINI {
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIReader::FieldSize> fallbackName;
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedAuthors;
        Array<char, INIReader::FieldSize> fallbackAuthor;
        Array<char, INIReader::FieldSize> version;
        Array<char, INIReader::FieldSize> defaultCourses;
    };

    struct CourseINI {
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIReader::FieldSize> fallbackName;
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedAuthors;
        Array<char, INIReader::FieldSize> fallbackAuthor;
        Array<char, INIReader::FieldSize> version;
        Array<char, INIReader::FieldSize> defaultCourseName;
        Array<char, INIReader::FieldSize> defaultMusicName;
    };

    CourseManager();

    OSThread &thread() override;
    void process() override;

    u32 raceCourseCount() const;
    u32 battleCourseCount() const;
    const Course &raceCourse(u32 index) const;
    const Course &battleCourse(u32 index) const;

    void addDefaultRaceCourses();
    void addDefaultBattleCourses();
    void addCustomPacksAndCourses(Array<char, 256> &path, Storage::NodeInfo &nodeInfo,
            Ring<u8, MaxCourseCount> &raceCourseIndices,
            Ring<u8, MaxCourseCount> &battleCourseIndices);
    void addCustomCourse(const Array<char, 256> &path, Ring<u8, MaxCourseCount> &raceCourseIndices,
            Ring<u8, MaxCourseCount> &battleCourseIndices);
    void addCustomRaceCourse(Ring<u8, MaxCourseCount> &courseIndices, const Array<char, 256> &path,
            const CustomCourse &course);
    void addCustomBattleCourse(Ring<u8, MaxCourseCount> &courseIndices,
            const Array<char, 256> &path, const CustomCourse &course);
    void addCustomRacePack(const Array<char, 256> &path, Ring<u8, MaxCourseCount> &courseIndices);
    void addCustomBattlePack(const Array<char, 256> &path, Ring<u8, MaxCourseCount> &courseIndices);
    void addCustomPack(const Array<char, 256> &path, Ring<u8, MaxCourseCount> &courseIndices,
            u32 defaultCourseOffset, u32 defaultCourseCount,
            Ring<CustomPack, MaxCustomPackCount> &packs, const char *type);
    void sortCustomRacePacksByName();
    void sortCustomBattlePacksByName();
    void addDefaultRacePacks();
    void addDefaultBattlePacks();
    void addDefaultPacks(u32 defaultCourseCount, u32 customCourseCount,
            Ring<DefaultPack, DefaultPackCount> &packs, const char *base, const char *type);
    void sortRacePackCoursesByName();
    void sortBattlePackCoursesByName();

    bool findPrefix(ZIPFile &zipFile, const char *filePath, Array<char, 128> &prefix) const;
    bool hashFile(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const;
    bool hashCourseFile(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const;
    bool loadCourseHash(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const;
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
    void *loadCourseFile(const char *zipPath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadCourseFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;

    static bool GetDefaultCourseID(const char *name, u32 &courseID);
    static void SortCustomPacksByName(Ring<CustomPack, MaxCustomPackCount> &packs);
    static bool ComparePacksByName(const Pack &a, const Pack &b);
    static bool CompareRaceCourseIndicesByName(const u32 &a, const u32 &b);
    static bool CompareBattleCourseIndicesByName(const u32 &a, const u32 &b);

    Array<u8, 64 * 1024> m_stack;
    OSThread m_thread;
    Ring<DefaultCourse, DefaultRaceCourseCount> m_defaultRaceCourses;
    Ring<CustomCourse, MaxCustomRaceCourseCount> m_customRaceCourses;
    Ring<DefaultCourse, DefaultBattleCourseCount> m_defaultBattleCourses;
    Ring<CustomCourse, MaxCustomBattleCourseCount> m_customBattleCourses;
    Ring<DefaultPack, DefaultPackCount> m_defaultRacePacks;
    Ring<CustomPack, MaxCustomPackCount> m_customRacePacks;
    Ring<DefaultPack, DefaultPackCount> m_defaultBattlePacks;
    Ring<CustomPack, MaxCustomPackCount> m_customBattlePacks;

    static CourseManager *s_instance;
};
