#include "MinimapConfigReader.hh"

extern "C" {
#include <stdio.h>
#include <string.h>
}

Optional<MinimapConfig> MinimapConfigReader::Read(ZIPFile &zip, const char *path) {
    MinimapConfigReader reader;
    return reader.read(zip, path);
}

MinimapConfigReader::MinimapConfigReader() {
    lwjson_stream_init(&m_jsp, Handle);
    void *userData = this;
    lwjson_stream_set_user_data(&m_jsp, userData);
}

MinimapConfigReader::~MinimapConfigReader() {}

Optional<MinimapConfig> MinimapConfigReader::read(ZIPFile &zip, const char *path) {
    Optional<MinimapConfig> minimapConfig;

    ZIPFile::Reader reader(zip, path);
    if (!reader.ok()) {
        return minimapConfig;
    }
    for (u32 offset = 0; offset < *reader.size();) {
        const u8 *chunk;
        u32 chunkSize;
        if (!reader.read(chunk, chunkSize)) {
            return minimapConfig;
        }
        while (chunkSize > 0) {
            lwjsonr_t result = lwjson_stream_parse(&m_jsp, chunk[0]);
            offset++;
            chunk++;
            chunkSize--;
            switch (result) {
            case lwjsonSTREAMWAITFIRSTCHAR:
            case lwjsonSTREAMINPROG:
                if (offset == *reader.size()) {
                    return minimapConfig;
                }
                break;
            case lwjsonSTREAMDONE:
                if (offset != *reader.size()) {
                    return minimapConfig;
                }
                break;
            default:
                return minimapConfig;
            }
        }
    }

    if (!m_startX || !m_startY || !m_endX || !m_endY || !m_orientation) {
        return minimapConfig;
    }

    minimapConfig.emplace();
    minimapConfig->box.start.x = *m_startX;
    minimapConfig->box.start.y = *m_startY;
    minimapConfig->box.end.x = *m_endX;
    minimapConfig->box.end.y = *m_endY;
    minimapConfig->orientation = *m_orientation;
    return minimapConfig;
}

void MinimapConfigReader::handle(lwjson_stream_type_t type) {
    if (m_jsp.stack_pos != 2) {
        return;
    }
    if (!lwjson_stack_seq_2(&m_jsp, 0, OBJECT, KEY)) {
        return;
    }
    if (type != LWJSON_STREAM_TYPE_NUMBER) {
        return;
    }

    const char *key = m_jsp.stack[1].meta.name;
    const char *number = m_jsp.data.prim.buff;
    f32 startX, startY, endX, endY;
    u32 orientation;
    if (!strcmp(key, "Top Left Corner X") && sscanf(number, "%f", &startX) == 1) {
        m_startX = startX;
    }
    if (!strcmp(key, "Top Left Corner Z") && sscanf(number, "%f", &startY) == 1) {
        m_startY = startY;
    }
    if (!strcmp(key, "Bottom Right Corner X") && sscanf(number, "%f", &endX) == 1) {
        m_endX = endX;
    }
    if (!strcmp(key, "Bottom Right Corner Z") && sscanf(number, "%f", &endY) == 1) {
        m_endY = endY;
    }
    if (!strcmp(key, "Orientation") && sscanf(number, "%lu", &orientation)) {
        m_orientation = orientation;
    }
}

void MinimapConfigReader::Handle(lwjson_stream_parser *jsp, lwjson_stream_type_t type) {
    void *userData = lwjson_stream_get_user_data(jsp);
    static_cast<MinimapConfigReader *>(userData)->handle(type);
}
