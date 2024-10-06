#pragma once

#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <game/KartLocale.hh>
#include <jsystem/JKRHeap.hh>

class StorageScanner : public Storage::Observer {
public:
    enum {
        INIFieldSize = 80,
    };

    void start();
    bool isLocked() const;
    bool lock();
    void unlock();

protected:
    class INI {
    public:
        struct Stream {
            UniquePtr<u8[]> ini;
            u32 iniSize;
            u32 iniOffset;
        };

        struct Field {
            const char *name;
            Array<char, INIFieldSize> *field;
        };

        struct LocalizedField {
            const char *name;
            Array<Array<char, INIFieldSize>, KartLocale::Language::Count> *fields;
        };

        INI(Stream &stream, u32 fieldCount, Field *fields, u32 localizedFieldCount,
                LocalizedField *localizedFields);
        ~INI();
        bool read();

    private:
        bool handleFields(const char *name, const char *value);
        bool handleLocalizedFields(const char *name, const char *value);

        static char *Read(char *str, int num, void *stream);
        static int Handle(void *user, const char *section, const char *name, const char *value);
        static void SetField(const char *value, Array<char, INIFieldSize> *field);

        Stream &m_stream;
        u32 m_fieldCount;
        Field *m_fields;
        u32 m_localizedFieldCount;
        LocalizedField *m_localizedFields;
    };

    StorageScanner();

    virtual OSThread &thread() = 0;
    virtual void process() = 0;

    const Array<u32, KartLocale::Language::Count> &languages() const;
    Array<char, INIFieldSize> &getLocalizedEntry(
            Array<Array<char, INIFieldSize>, KartLocale::Language::Count> &localizedEntries,
            Array<char, INIFieldSize> &fallbackEntry);

    static void *Run(void *param);

private:
    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    void onChange(const char *prefix);
    void notify();
    void *run();

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;
    bool m_currIsLocked;
    bool m_nextIsLocked;
    bool m_hasChanged;
    Array<u32, KartLocale::Language::Count> m_languages;
};
