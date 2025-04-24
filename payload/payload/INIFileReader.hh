#pragma once

#include "payload/INIReader.hh"

#include <cube/storage/Storage.hh>

class INIFileReader : private INIReader {
public:
    static bool Read(u32 fieldCount, Field *fields, u32 localizedFieldCount,
            LocalizedField *localizedFields, const char *path);

private:
    INIFileReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
            LocalizedField *localizedFields, const char *path);
    ~INIFileReader();
    bool read();

    char *read(char *str, int num) override;

    bool m_ok;
    Storage::FileHandle m_file;
    u32 m_size;
    u32 m_offset;
};
