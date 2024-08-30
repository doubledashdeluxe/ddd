#include "StorageScanner.hh"

#include "payload/FileLoader.hh"
#include "payload/Lock.hh"

extern "C" {
#include <stdio.h>
#include <string.h>
#include <strings.h>
}

void StorageScanner::start() {
    notify();
    OSResumeThread(&m_thread);
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

StorageScanner::StorageScanner(u8 threadPriority)
    : m_currIsLocked(false), m_nextIsLocked(false), m_hasChanged(true) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(),
            threadPriority, 0);
}

UniquePtr<char[]> &StorageScanner::getLocalizedEntry(
        Array<UniquePtr<char[]>, KartLocale::Language::Count> &localizedEntries,
        UniquePtr<char[]> &fallbackEntry) {
    for (u32 i = 0; i < KartLocale::Language::Count; i++) {
        if (localizedEntries[m_languages[i]].get()) {
            return localizedEntries[m_languages[i]];
        }
    }
    return fallbackEntry;
}

void *StorageScanner::loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
        u32 *size) const {
    bool ok;
    ZIPFile zipFile(zipPath, ok);
    if (!ok) {
        return nullptr;
    }
    return loadFile(zipFile, filePath, heap, size);
}

void *StorageScanner::loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
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

void *StorageScanner::loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
        JKRHeap *heap, u32 *size) const {
    bool ok;
    ZIPFile zipFile(zipPath, ok);
    if (!ok) {
        return nullptr;
    }
    return loadLocalizedFile(zipFile, prefix, suffix, heap, size);
}

void *StorageScanner::loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix,
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

void *StorageScanner::loadLocalizedFile(const char *prefix, const char *suffix, JKRHeap *heap,
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

char *StorageScanner::ReadINI(char *str, int num, void *stream) {
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

bool StorageScanner::HandleINIFields(const char *name, const char *value, u32 fieldCount,
        const INIField *fields, JKRHeap *heap) {
    for (u32 i = 0; i < fieldCount; i++) {
        if (strcasecmp(name, fields[i].name)) {
            continue;
        }
        if (SetINIField(value, fields[i].field, heap)) {
            return true;
        }
    }
    return false;
}

bool StorageScanner::HandleLocalizedINIFields(const char *name, const char *value, u32 fieldCount,
        const LocalizedINIField *fields, JKRHeap *heap) {
    for (u32 i = 0; i < fieldCount; i++) {
        if (strncasecmp(name, fields[i].name, strlen(fields[i].name))) {
            continue;
        }
        for (u32 j = 0; j < KartLocale::Language::Count; j++) {
            if (strcasecmp(name + strlen(fields[i].name), KartLocale::GetLanguageName(j))) {
                continue;
            }
            if (SetINIField(value, &(*fields[i].fields)[j], heap)) {
                return true;
            }
        }
    }
    return false;
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

void *StorageScanner::Run(void *param) {
    return reinterpret_cast<StorageScanner *>(param)->run();
}

bool StorageScanner::SetINIField(const char *value, UniquePtr<char[]> *field, JKRHeap *heap) {
    u32 valueLength = strlen(value);
    field->reset(new (heap, 0x4) char[valueLength + 1]);
    if (!field->get()) {
        return false;
    }
    field->get()[valueLength] = '\0';
    memcpy(field->get(), value, valueLength);
    return true;
}
