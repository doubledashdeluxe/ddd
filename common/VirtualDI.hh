#pragma once

#include "common/Array.hh"
#include "common/storage/Storage.hh"

class VirtualDI {
public:
    static void Init();
    static bool IsInit();
    static bool Mount();
    static bool Read(void *dst, u32 size, u32 offset);

private:
    static bool Mount(Array<char, 256> &path, Storage::DirHandle &dir);
    static bool Mount(Array<char, 256> &path, Storage::FileHandle &file);
    static bool ReadDiscID(Storage::FileHandle &file);

    static Storage::FileHandle *s_file;
};
