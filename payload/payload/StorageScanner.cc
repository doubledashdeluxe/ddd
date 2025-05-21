#include "StorageScanner.hh"

#include "payload/Lock.hh"

extern "C" {
#include <string.h>
}

void StorageScanner::start() {
    notify();
    OSResumeThread(&thread());
    OSReceiveMessage(&m_initQueue, nullptr, OS_MESSAGE_BLOCK);
}

bool StorageScanner::isLocked() const {
    return m_currIsLocked;
}

bool StorageScanner::lock() {
    Lock<NoInterrupts> lock;
    if (m_currIsLocked) {
        return true;
    }
    m_nextIsLocked = true;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    return false;
}

void StorageScanner::unlock() {
    Lock<NoInterrupts> lock;
    if (!m_nextIsLocked && !m_currIsLocked) {
        return;
    }
    m_nextIsLocked = false;
    m_currIsLocked = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

StorageScanner::StorageScanner()
    : m_currIsLocked(false), m_nextIsLocked(false), m_hasChanged(true) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
}

const Array<u32, KartLocale::Language::Count> &StorageScanner::languages() const {
    return m_languages;
}

Array<char, INIReader::FieldSize> &StorageScanner::getLocalizedEntry(
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> &localizedEntries,
        Array<char, INIReader::FieldSize> &fallbackEntry) {
    for (u32 i = 0; i < KartLocale::Language::Count; i++) {
        if (strlen(localizedEntries[m_languages[i]].values()) != 0) {
            return localizedEntries[m_languages[i]];
        }
    }
    return fallbackEntry;
}

void *StorageScanner::Run(void *param) {
    return static_cast<StorageScanner *>(param)->run();
}

void StorageScanner::onAdd(const char *prefix) {
    onChange(prefix);
}

void StorageScanner::onRemove(const char *prefix) {
    onChange(prefix);
}

void StorageScanner::onChange(const char *prefix) {
    if (!strcmp(prefix, "main:")) {
        notify();
    }
}

void StorageScanner::notify() {
    Lock<NoInterrupts> lock;
    m_hasChanged = true;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *StorageScanner::run() {
    m_languages = (u32[KartLocale::Language::Count]){
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

        process();
    }
}
