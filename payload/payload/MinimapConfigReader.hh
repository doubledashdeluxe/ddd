#pragma once

#include "payload/ZIPFile.hh"

#include <game/MinimapConfig.hh>
extern "C" {
#include <lwjson/lwjson.h>
}
#include <portable/Optional.hh>

class MinimapConfigReader {
public:
    static Optional<MinimapConfig> Read(ZIPFile &zip, const char *path);

private:
    MinimapConfigReader();
    ~MinimapConfigReader();
    Optional<MinimapConfig> read(ZIPFile &zip, const char *path);
    void handle(lwjson_stream_type_t type);

    static void Handle(lwjson_stream_parser *jsp, lwjson_stream_type_t type);

    lwjson_stream_parser m_jsp;
    Optional<f32> m_startX;
    Optional<f32> m_startY;
    Optional<f32> m_endX;
    Optional<f32> m_endY;
    Optional<u32> m_orientation;
};
