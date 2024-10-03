#include "StorageScanner.hh"

#include "payload/FileLoader.hh"
#include "payload/Lock.hh"

extern "C" {
#include <inih/ini.h>

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

StorageScanner::INI::INI(Stream &stream, u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields)
    : m_stream(stream), m_fieldCount(fieldCount), m_fields(fields),
      m_localizedFieldCount(localizedFieldCount), m_localizedFields(localizedFields) {}

StorageScanner::INI::~INI() {}

bool StorageScanner::INI::read() {
    for (u32 i = 0; i < m_fieldCount; i++) {
        m_fields[i].field[0] = '\0';
    }
    for (u32 i = 0; i < m_localizedFieldCount; i++) {
        for (u32 j = 0; j < KartLocale::Language::Count; j++) {
            (*m_localizedFields[i].fields)[j][0] = '\0';
        }
    }

    return !ini_parse_stream(Read, &m_stream, Handle, this);
}

bool StorageScanner::INI::handleFields(const char *name, const char *value) {
    for (u32 i = 0; i < m_fieldCount; i++) {
        if (!strcasecmp(name, m_fields[i].name)) {
            SetField(value, m_fields[i].field);
            return true;
        }
    }
    return false;
}

bool StorageScanner::INI::handleLocalizedFields(const char *name, const char *value) {
    for (u32 i = 0; i < m_localizedFieldCount; i++) {
        if (strncasecmp(name, m_localizedFields[i].name, strlen(m_localizedFields[i].name))) {
            continue;
        }
        for (u32 j = 0; j < KartLocale::Language::Count; j++) {
            if (!strcasecmp(name + strlen(m_localizedFields[i].name),
                        KartLocale::GetLanguageName(j))) {
                SetField(value, &(*m_localizedFields[i].fields)[j]);
                return true;
            }
        }
    }
    return false;
}

char *StorageScanner::INI::Read(char *str, int num, void *stream) {
    INI::Stream *iniStream = reinterpret_cast<INI::Stream *>(stream);
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

int StorageScanner::INI::Handle(void *user, const char *section, const char *name,
        const char *value) {
    if (strcmp(section, "Config")) {
        return 1;
    }

    INI *ini = reinterpret_cast<INI *>(user);
    if (ini->handleFields(name, value)) {
        return 1;
    }
    if (ini->handleLocalizedFields(name, value)) {
        return 1;
    }

    return 1;
}

void StorageScanner::INI::SetField(const char *value, Array<char, INIFieldSize> *field) {
    snprintf(field->values(), field->count(), "%s", value);
}

StorageScanner::StorageScanner(u8 threadPriority)
    : m_currIsLocked(false), m_nextIsLocked(false), m_hasChanged(true) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(),
            threadPriority, 0);
}

Array<char, StorageScanner::INIFieldSize> &StorageScanner::getLocalizedEntry(
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> &localizedEntries,
        Array<char, INIFieldSize> &fallbackEntry) {
    for (u32 i = 0; i < KartLocale::Language::Count; i++) {
        if (strlen(localizedEntries[m_languages[i]].values()) != 0) {
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
