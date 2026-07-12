#pragma once

#include "game/PrintWindow.hh"
#include "game/Scene.hh"

#include <cube/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <portable/UniquePtr.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStateUpdateWriteInfo.hh>

class SceneUpdate
    : public Scene
    , private ClientReadHandler {
public:
    SceneUpdate(JKRArchive *archive, JKRHeap *heap);
    ~SceneUpdate();
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneUpdate::*State)();

    bool clientStateServer(const ClientStateServerReadInfo &readInfo) override;
    bool clientStateUpdate(const ClientStateUpdateReadInfo &readInfo) override;
    void clientStateError() override;

    void wait();
    void info();
    void data();
    void completed();
    void nextScene();
    void reboot();

    void stateWait();
    void stateInfo();
    void stateData();
    void stateCompleted();
    void stateNextScene();
    void stateReboot();

    bool isWindowClosed() const;
    bool isWindowOpen() const;
    bool openWindow();
    bool closeWindow();

    void update();
    bool download(Storage::FileHandle &file);
    bool verify(Storage::FileHandle &file);
    bool install(Storage::FileHandle &file);

    void report(const char *text);
    void report(const char *text, u32 size, u32 totalSize, s64 start);

    static void *Update(void *param);

    static bool Rename(const char *srcPath, const char *dstPath);
    static bool Remove(const char *path, u32 mode);
    static bool RemoveAll(const char *path, u32 mode);

    JKRHeap *m_heap;
    State m_state;
    PrintWindow m_window;
    u8 m_windowFrame;
    bool m_canceled;
    bool m_completed;
    Mutex m_mutex;
    ClientStateUpdateReadInfo m_readInfo;
    ClientStateUpdateWriteInfo m_writeInfo;
    Array<char, MaxVersionLength + 1> m_version;
    u32 m_size;
    u32 m_alignedSize;
    u32 m_chunkCount;
    u32 m_downloadedChunkCount;
    u32 m_chunkIndices[256];
    u32 m_chunkIndex;
    char m_text[256];
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    UniquePtr<u8[]> m_stack;
    OSThread m_thread;

    static const char ZIPPath[];
    static const char UpdatePath[];
    static const char BackupPath[];
    static const char AppPath[];
};
