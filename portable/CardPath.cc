#include "CardPath.hh"

#include <portable/Bytes.hh>

extern "C" {
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
}

Array<char, 256> CardPath::Get(const char *dirPath, const char fileName[32], const char gameName[4],
        const char company[2]) {
    Array<char, 256> path;
    if (!strcmp(fileName, "MarioKart Double Dash!!")) {
        snprintf(path.values(), path.count(), "%s/%.2s-%.4s-%s.gci", dirPath, company, gameName,
                fileName);
    } else {
        Array<u8, 0x15> giRecord;
        for (u32 i = 0; i < giRecord.count(); i += 3) {
            Array<u8, 4> values;
            for (u32 j = 0; j < values.count(); j++) {
                values[j] = fileName[1 + i / 3 * 4 + j] - '!';
            }
            giRecord[i + 0] = values[0] << 2 | values[1] >> 4;
            giRecord[i + 1] = values[1] << 4 | values[2] >> 2;
            giRecord[i + 2] = values[2] << 6 | values[3] >> 0;
        }
        u8 courseID = Bytes::ReadBE<u8>(giRecord.values(), 0x03);
        u32 time = Bytes::ReadBE<u32>(giRecord.values(), 0x08);
        u32 minutes = time / (60 * 1000);
        u32 seconds = time / 1000 % 60;
        u32 milliseconds = time % 1000;
        Array<char, 4> tag('\0');
        memcpy(tag.values(), &giRecord.values()[0x04], tag.count() - 1);
        Array<u8, 2> charIDs;
        for (u32 i = 0; i < charIDs.count(); i++) {
            charIDs[i] = Bytes::ReadBE<u8>(giRecord.values(), 0x00 + i);
        }
        u8 kartID = Bytes::ReadBE<u8>(giRecord.values(), 0x02);
        snprintf(path.values(), path.count(),
                "%s/%.2s-%.4s-%02x-%" PRIu32 "m%02" PRIu32 "s%03" PRIu32
                "-%.3s-%02x-%02x-%02x-%.2s.gci",
                dirPath, company, gameName, courseID, minutes, seconds, milliseconds, tag.values(),
                charIDs[0], charIDs[1], kartID, &fileName[0x1d]);
    }
    return path;
}
