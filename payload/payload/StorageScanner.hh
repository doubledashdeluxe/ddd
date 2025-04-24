#pragma once

#include "payload/INIReader.hh"

#include <cube/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <game/KartLocale.hh>

class StorageScanner : public Storage::Observer {
public:
    void start();
    bool isLocked() const;
    bool lock();
    void unlock();

protected:
    StorageScanner();

    virtual OSThread &thread() = 0;
    virtual void process() = 0;

    const Array<u32, KartLocale::Language::Count> &languages() const;
    Array<char, INIReader::FieldSize> &getLocalizedEntry(
            Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> &localizedEntries,
            Array<char, INIReader::FieldSize> &fallbackEntry);

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
