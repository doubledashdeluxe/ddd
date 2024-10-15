#include "INIFileReader.hh"

bool INIFileReader::Read(u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields, const char *path) {
    INIFileReader reader(fieldCount, fields, localizedFieldCount, localizedFields, path);
    return reader.read();
}

INIFileReader::INIFileReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields, const char *path)
    : INIReader(fieldCount, fields, localizedFieldCount, localizedFields), m_ok(true),
      m_file(path, Storage::Mode::Read) {}

INIFileReader::~INIFileReader() {}

bool INIFileReader::read() {
    u64 size;
    if (!m_file.size(size)) {
        return false;
    }
    if (size > UINT32_MAX) {
        return false;
    }
    m_size = size;
    m_offset = 0;
    return INIReader::read() && m_ok;
}

char *INIFileReader::read(char *str, int num) {
    if (!m_ok || m_offset == m_size || num < 2) {
        return nullptr;
    }

    char *s;
    for (s = str; num > 1 && m_offset < m_size; num--) {
        char c;
        if (!m_file.read(&c, sizeof(c), m_offset++)) {
            m_ok = false;
            return nullptr;
        }
        *s++ = c;
        if (c == '\n') {
            break;
        }
    }
    *s = '\0';
    return str;
}
