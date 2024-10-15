#pragma once

#include "payload/INIReader.hh"
#include "payload/ZIPFile.hh"

class INIZIPFileReader : private INIReader {
public:
    static bool Read(u32 fieldCount, Field *fields, u32 localizedFieldCount,
            LocalizedField *localizedFields, ZIPFile &zipFile, const char *filePath);

private:
    INIZIPFileReader(u32 fieldCount, Field *fields, u32 localizedFieldCount,
            LocalizedField *localizedFields, ZIPFile &zipFile, const char *filePath);
    ~INIZIPFileReader();
    bool read();

    char *read(char *str, int num) override;

    bool m_ok;
    ZIPFile::Reader m_reader;
    u32 m_size;
    const u8 *m_chunk;
    u32 m_chunkSize;
};
