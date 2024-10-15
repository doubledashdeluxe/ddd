#include "INIZIPFileReader.hh"

bool INIZIPFileReader::Read(u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields, ZIPFile &zipFile, const char *filePath) {
    INIZIPFileReader reader(fieldCount, fields, localizedFieldCount, localizedFields, zipFile,
            filePath);
    return reader.read();
}

INIZIPFileReader::INIZIPFileReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields, ZIPFile &zipFile, const char *filePath)
    : INIReader(fieldCount, fields, localizedFieldCount, localizedFields), m_ok(true),
      m_reader(zipFile, filePath) {}

INIZIPFileReader::~INIZIPFileReader() {}

bool INIZIPFileReader::read() {
    if (!m_reader.ok()) {
        return false;
    }

    m_size = *m_reader.size();
    m_chunkSize = 0;
    return INIReader::read() && m_ok;
}

char *INIZIPFileReader::read(char *str, int num) {
    if (!m_ok || m_size == 0 || num < 2) {
        return nullptr;
    }

    char *s;
    for (s = str; num > 1 && m_size > 0; num--) {
        while (m_chunkSize == 0) {
            if (!m_reader.read(m_chunk, m_chunkSize)) {
                m_ok = false;
                return nullptr;
            }
        }
        char c = m_chunk[0];
        m_chunk++;
        m_chunkSize--;
        m_size--;
        *s++ = c;
        if (c == '\n') {
            break;
        }
    }
    *s = '\0';
    return str;
}
