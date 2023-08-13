#pragma once

#include "common/Types.hh"

#ifdef PAYLOAD
typedef struct OSContext OSContext;
#endif

namespace IOS {

class Resource {
public:
    struct IoctlvPair {
        void *data;
        u32 size;
    };

    Resource(s32 fd);
    Resource(const char *path, bool read = false, bool write = false);
    ~Resource();
    s32 ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize);
    s32 ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs);
    bool ok() const;

#ifdef PAYLOAD
    static void Init();
#endif

private:
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
        u8 user[0x40 - 0x20];
    };
    static_assert(sizeof(Request) == 0x40);

    Resource(const Resource &);
    Resource &operator=(const Resource &);
    s32 open(const char *path, bool read, bool write);
    s32 close();

    static void Sync(Request &request);
#ifdef PAYLOAD
    static void HandleInterrupt(s16 interrupt, OSContext *context);
#endif

protected:
    s32 m_fd;
};

} // namespace IOS
