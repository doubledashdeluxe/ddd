#pragma once

#include "common/Types.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSThread.h>
}
#endif

namespace IOS {

class Mode {
public:
    enum {
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1,
        Both = Write | Read,
    };

private:
    Mode();
};

class Resource {
public:
    struct IoctlvPair {
        void *data;
        u32 size;
    };

    Resource(s32 fd);
    Resource(const char *path, u32 mode);
    ~Resource();
    s32 ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize);
    s32 ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs);
    bool ok() const;

#ifdef PAYLOAD
    static void Init();
#endif

protected:
    class Command {
    public:
        enum {
            Open = 1,
            Close = 2,
            Read = 3,
            Write = 4,
            Seek = 5,
            Ioctl = 6,
            Ioctlv = 7,
        };

    private:
        Command();
    };

    struct Request {
        u32 command;
        s32 result;
        s32 fd;
        union {
            struct {
                u32 path;
                u32 mode;
            } open;
            struct {
                u32 output;
                u32 outputSize;
            } read;
            struct {
                u32 input;
                u32 inputSize;
            } write;
            struct {
                u32 ioctl;
                u32 input;
                u32 inputSize;
                u32 output;
                u32 outputSize;
            } ioctl;
            struct {
                u32 ioctlv;
                u32 inputCount;
                u32 outputCount;
                u32 pairs;
            } ioctlv;
        };
        struct {
#ifdef PAYLOAD
            Request *next;
            OSThreadQueue queue;
            u8 _0c[0x20 - 0x0c];
#else
            u8 _00[0x20 - 0x00];
#endif
        } user;
    };
    static_assert(sizeof(Request) == 0x40);

    static void Sync(Request &request);

private:
    Resource(const Resource &);
    Resource &operator=(const Resource &);
    s32 open(const char *path, u32 mode);
    s32 close();

#ifdef PAYLOAD
    static void HandleInterrupt(s16 interrupt, OSContext *context);
#endif

protected:
    s32 m_fd;

private:
#ifdef PAYLOAD
    static Request *s_request;
#endif
};

} // namespace IOS
