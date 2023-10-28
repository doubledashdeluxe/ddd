#pragma once

#include "jsystem/JASTaskThread.hh"

#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>
}
#include <payload/Replace.hh>

class JASAramStream {
public:
    REPLACE JASAramStream();

    REPLACE bool prepare(s32 entrynum, s32 aramBlockCount);

private:
    class Format {
    public:
        enum {
            ADPCM = 0,
            PCM = 1,
        };

    private:
        Format();
    };

    struct HeaderLoadTask {
        JASAramStream *aramStream;
        s32 entrynum;
        u32 aramBufferSize;
        s32 aramBlockCount;
    };

    struct LoadTask {
        JASAramStream *aramStream;
        u32 _4;
        u32 _8;
    };

    bool headerLoad(s32 entrynum, u32 aramBufferSize, s32 aramBlockCount);
    REPLACE bool load();
    bool openFile(s32 entrynum);

    static s32 ChannelProc(void *userData);
    static void HeaderLoad(void *userData);
    static void FirstLoad(void *userData);
    static void REPLACED(Finish)(void *userData);
    REPLACE static void Finish(void *userData);

    OSMessageQueue m_controlQueue;
    OSMessageQueue m_stateQueue;
    u8 _040[0x0cc - 0x040];
    Storage::FileHandle m_file; // Modified
    u8 _unused[sizeof(DVDFileInfo) - sizeof(Storage::FileHandle)];
    u32 m_aramLoadBlockCount;
    u32 m_aramCurrentBlock;
    u32 m_currentBlock;
    volatile bool m_wasCanceled;
    volatile u32 m_aramFreeBlockCount;
    u8 _11c[0x130 - 0x11c];
    s16 m_coefs[2][6];
    u32 m_aramBuffer;
    u32 m_aramBufferSize;
    u8 _150[0x158 - 0x150];
    u16 m_format;
    u16 m_channelCount;
    u32 m_aramChannelBlockCount;
    u32 m_aramBlockCount;
    u32 m_sampleRate;
    bool m_hasLoop;
    u32 m_loopStart;
    u32 m_loopEnd;
    f32 m_volume;
    u8 _178[0x1e8 - 0x178];

    static JASTaskThread *s_loadThread;
    static u8 *s_readBuffer;
    static u32 s_blockSize;
    static bool s_fatalErrorFlag;
    static Array<char, 256> s_name; // Added
};
static_assert(sizeof(JASAramStream) == 0x1e8);
