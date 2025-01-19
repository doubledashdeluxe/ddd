#pragma once

#include "common/Types.hh"

struct OSContext;

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
    Resource(const char *path, u32 mode, bool isBlocking = true);
    ~Resource();
    s32 ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize);
    s32 ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs);
    bool ioctlvReboot(u32 ioctlv, u32 inputCount, IoctlvPair *pairs);
    bool ok() const;

    static void Init();

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
                uintptr_t output;
                size_t outputSize;
            } read;
            struct {
                uintptr_t input;
                size_t inputSize;
            } write;
            struct {
                u32 ioctl;
                uintptr_t input;
                size_t inputSize;
                uintptr_t output;
                size_t outputSize;
            } ioctl;
            struct {
                u32 ioctlv;
                u32 inputCount;
                u32 outputCount;
                uintptr_t pairs;
            } ioctlv;
        };
        struct {
            Request *next;
            u8 _04[0x20 - 0x04];
        } user;
    };
    size_assert(Request, 0x40);

    static void Sync(Request &request);
    static bool SyncReboot(Request &request);

private:
    Resource(const Resource &);
    Resource &operator=(const Resource &);
    s32 open(const char *path, u32 mode);
    s32 close();

    static void HandleInterrupt(s16 interrupt, OSContext *context);

protected:
    s32 m_fd;

private:
    static Request *s_request;
};

} // namespace IOS
