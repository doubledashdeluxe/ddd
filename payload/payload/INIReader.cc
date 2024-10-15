#include "INIReader.hh"

extern "C" {
#include <inih/ini.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
}

INIReader::INIReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
        LocalizedField *localizedFields)
    : m_fieldCount(fieldCount), m_fields(fields), m_localizedFieldCount(localizedFieldCount),
      m_localizedFields(localizedFields) {}

INIReader::~INIReader() {}

bool INIReader::read() {
    for (u32 i = 0; i < m_fieldCount; i++) {
        m_fields[i].field[0] = '\0';
    }
    for (u32 i = 0; i < m_localizedFieldCount; i++) {
        for (u32 j = 0; j < KartLocale::Language::Count; j++) {
            (*m_localizedFields[i].fields)[j][0] = '\0';
        }
    }

    return !ini_parse_stream(Read, this, Handle, this);
}

bool INIReader::handleFields(const char *name, const char *value) {
    for (u32 i = 0; i < m_fieldCount; i++) {
        if (!strcasecmp(name, m_fields[i].name)) {
            SetField(value, m_fields[i].field);
            return true;
        }
    }
    return false;
}

bool INIReader::handleLocalizedFields(const char *name, const char *value) {
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

int INIReader::handle(const char *section, const char *name, const char *value) {
    if (strcmp(section, "Config")) {
        return 1;
    }

    if (handleFields(name, value)) {
        return 1;
    }
    if (handleLocalizedFields(name, value)) {
        return 1;
    }

    return 1;
}

char *INIReader::Read(char *str, int num, void *stream) {
    return reinterpret_cast<INIReader *>(stream)->read(str, num);
}

int INIReader::Handle(void *user, const char *section, const char *name, const char *value) {
    return reinterpret_cast<INIReader *>(user)->handle(section, name, value);
}

void INIReader::SetField(const char *value, Array<char, FieldSize> *field) {
    snprintf(field->values(), field->count(), "%s", value);
}
