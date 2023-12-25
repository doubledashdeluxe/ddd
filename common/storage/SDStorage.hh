#pragma once

#include "common/ios/Resource.hh"
#include "common/storage/FATStorage.hh"

#ifdef PAYLOAD
#include <payload/Mutex.hh>
#endif

class SDStorage : private IOS::Resource, private FATStorage {
public:
    static void Init();

private:
    typedef void (SDStorage::*PollCallback)();

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
        CardHandle(SDStorage *storage);
        ~CardHandle();
        bool ok() const;

    private:
        CardHandle(const CardHandle &);
        CardHandle &operator=(const CardHandle &);

        SDStorage *m_storage;
    };

    SDStorage();

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    u32 sectorSize() override;
    bool read(u32 firstSector, u32 sectorCount, void *buffer) override;
    bool write(u32 firstSector, u32 sectorCount, const void *buffer) override;
    bool erase(u32 firstSector, u32 sectorCount) override;
    bool sync() override;

    bool transfer(bool isWrite, u32 firstSector, u32 sectorCount, void *buffer);
#ifdef PAYLOAD
    void *run();
    bool waitFor(Status status);
    void pollAdd();
    void pollRemove();
#endif

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

#ifdef PAYLOAD
    static void *Run(void *param);
#endif

    PollCallback m_pollCallback;
    u16 m_rca;
    bool m_isSDHC;
    alignas(0x20) Array<u8, 0x4000> m_buffer;
#ifdef PAYLOAD
    Mutex m_mutex;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
#endif

    static const u32 SectorSize;

    static SDStorage *s_instance;
};
