#pragma once

#include "payload/ZIPFile.hh"

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
    void start();
    bool isLocked() const;
    bool lock();
    void unlock();

protected:
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

    StorageScanner(u8 threadPriority);

    virtual void process() = 0;

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

    static char *ReadINI(char *str, int num, void *stream);
    static bool HandleINIFields(const char *name, const char *value, u32 fieldCount,
            const INIField *fields, JKRHeap *heap);
    static bool HandleLocalizedINIFields(const char *name, const char *value, u32 fieldCount,
            const LocalizedINIField *fields, JKRHeap *heap);

private:
    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    void onChange(const char *prefix);
    void notify();
    void *run();

    static void *Run(void *param);
    static bool SetINIField(const char *value, UniquePtr<char[]> *field, JKRHeap *heap);

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;
    Array<u8, 128 * 1024> m_stack;
    OSThread m_thread;
    bool m_currIsLocked;
    bool m_nextIsLocked;
    bool m_hasChanged;
    Array<u32, KartLocale::Language::Count> m_languages;
};
