#include "VirtualDI.hh"

#include "common/Arena.hh"
#include "common/DCache.hh"
#include "common/DiscID.hh"
#include "common/Log.hh"

extern "C" {
#include <stdio.h>
#include <string.h>
}

void VirtualDI::Init() {
    s_file = new (MEM1Arena::Instance(), -0x4) Storage::FileHandle;
}

bool VirtualDI::IsInit() {
    return s_file;
}

bool VirtualDI::Mount() {
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/games");
    Storage::DirHandle dir(path.values());
    return Mount(path, dir);
}

bool VirtualDI::Read(void *dst, u32 size, u32 offset) {
    if (!s_file->read(dst, size, offset)) {
        return false;
    }
    DCache::Flush(dst, size);
    return true;
}

bool VirtualDI::Mount(Array<char, 256> &path, Storage::DirHandle &dir) {
    u32 length = strlen(path.values());
    for (Storage::NodeInfo nodeInfo; dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            Storage::DirHandle dir(path.values());
            if (Mount(path, dir)) {
                return true;
            }
        } else {
            if (!s_file->open(path.values(), Storage::Mode::Read)) {
                continue;
            }
            if (Mount(path, *s_file)) {
                return true;
            }
            s_file->close();
        }
    }
    path[length] = '\0';
    return false;
}

bool VirtualDI::Mount(Array<char, 256> &path, Storage::FileHandle &file) {
    u64 fileSize;
    if (!file.size(fileSize)) {
        return false;
    }
    if (fileSize != 1459978240) {
        return false;
    }

    if (!ReadDiscID(file)) {
        return false;
    }

    INFO("Mario Kart: Double Dash!! disc image found (game id %.4s,", DiscID::Get().gameID);
    INFO("path: %s).", path.values());
    return true;
}

Storage::FileHandle *VirtualDI::s_file = nullptr;
