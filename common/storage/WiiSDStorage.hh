#pragma once

#include "common/ios/Resource.hh"
#include "common/storage/FATStorage.hh"

extern "C" {
struct OSMessageQueue;
}

class WiiSDStorage
    : private IOS::Resource
    , private FATStorage {
public:
    WiiSDStorage();
    ~WiiSDStorage();

    static void Init();

private:
    typedef void (WiiSDStorage::*PollCallback)();

    class Ioctl {
    public:
        enum {
            WriteHCR = 0x1,
            ReadHCR = 0x2,
            ResetCard = 0x4,
            SetClock = 0x6,
            SendCommand = 0x7,
            GetStatus = 0xb,
        };

    private:
        Ioctl();
    };

    class Ioctlv {
    public:
        enum {
            SendCommand = 0x7,
        };

    private:
        Ioctlv();
    };

    struct RegOp {
        u32 reg;
        u32 _04;
        u32 _08;
        u32 size;
        u8 val[4];
        u32 _14;
    };
    size_assert(RegOp, 0x18);

    class HCR {
    public:
        enum {
            HostControl1 = 0x28,
        };

    private:
        HCR();
    };

    struct HostControl1 {
        u8 : 6;
        u8 dataTransferWidth : 1;
        u8 : 1;
    };
    size_assert(HostControl1, 0x1);

    class Command {
    public:
        enum {
            Select = 7,
            SetBlocklen = 16,
            ReadMultipleBlock = 18,
            WriteMultipleBlock = 25,
            AppCmd = 55,
        };

    private:
        Command();
    };

    class AppCommand {
    public:
        enum {
            SetBusWidth = 6,
        };

    private:
        AppCommand();
    };

    class VirtualCommand {
    public:
        enum {
            Wait = 64,
            Cancel = 65,
        };

    private:
        VirtualCommand();
    };

    class ResponseType {
    public:
        enum {
            R1 = 1,
            R1B = 2,
        };

    private:
        ResponseType();
    };

    struct Request {
        u32 command;
        u32 commandType;
        u32 responseType;
        u32 arg;
        u32 blockCount;
        u32 blockSize;
        void *buffer;
        u32 isDma;
        u32 _20;
    };
    size_assert(Request, 0x24);

    struct Status {
        u32 : 11;
        bool isSDHC : 1;
        u32 : 3;
        bool isMemory : 1;
        u32 : 14;
        bool wasRemoved : 1;
        bool wasAdded : 1;
    };
    size_assert(Status, 0x4);

    class CardHandle {
    public:
        CardHandle(WiiSDStorage *storage);
        ~CardHandle();
        bool ok() const;

    private:
        CardHandle(const CardHandle &);
        CardHandle &operator=(const CardHandle &);

        WiiSDStorage *m_storage;
    };

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    u32 sectorSize() override;
    bool read(u32 firstSector, u32 sectorCount, void *buffer) override;
    bool write(u32 firstSector, u32 sectorCount, const void *buffer) override;
    bool erase(u32 firstSector, u32 sectorCount) override;
    bool sync() override;

    bool transfer(bool isWrite, u32 firstSector, u32 sectorCount, void *buffer);
    void *run();
    bool waitFor(Status status);
    void pollAdd();
    void pollRemove();

    bool enable4BitBus();
    bool setCardBlockLength(u32 blockLength);
    bool enableCard4BitBus();

    bool writeHCR(u8 reg, u8 size, const void *val);
    bool readHCR(u8 reg, u8 size, void *val);
    bool resetCard();
    bool setClock(u32 clock);
    bool sendCommand(u32 command, u32 commandType, u32 responseType, u32 arg, u32 blockCount,
            u32 blockSize, void *buffer, u32 *response);
    bool getStatus(Status &status);

    static void *Run(void *param);

    PollCallback m_pollCallback;
    u16 m_rca;
    bool m_isSDHC;

    static const u32 SectorSize;

    static Array<u8, 0x4000> *s_buffer;
    static WiiSDStorage *s_instance;
    static Mutex *s_mutex;
    static OSMessageQueue s_queue;
};
