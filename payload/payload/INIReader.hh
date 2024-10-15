#pragma once

#include <common/Array.hh>
#include <game/KartLocale.hh>

class INIReader {
public:
    enum {
        FieldSize = 80,
    };

    struct Field {
        const char *name;
        Array<char, FieldSize> *field;
    };

    struct LocalizedField {
        const char *name;
        Array<Array<char, FieldSize>, KartLocale::Language::Count> *fields;
    };

    INIReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
            LocalizedField *localizedFields);
    ~INIReader();
    bool read();

private:
    virtual char *read(char *str, int num) = 0;

    int handle(const char *section, const char *name, const char *value);

    bool handleFields(const char *name, const char *value);
    bool handleLocalizedFields(const char *name, const char *value);

    static char *Read(char *str, int num, void *stream);
    static int Handle(void *user, const char *section, const char *name, const char *value);
    static void SetField(const char *value, Array<char, FieldSize> *field);

    u32 m_fieldCount;
    Field *m_fields;
    u32 m_localizedFieldCount;
    LocalizedField *m_localizedFields;
};
