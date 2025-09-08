#pragma once

#include "cube/EXI.hh"
#include "cube/storage/SDStorage.hh"

class EXISDStorage : private SDStorage {
public:
    EXISDStorage(u32 channel);
    ~EXISDStorage();

    static void Init();

private:
    typedef bool (EXISDStorage::*TransferFunc)(u32 firstSector, u32 sectorCount, void *buffer);

    struct Transfer {
        TransferFunc func;
        u32 firstSector;
        u32 sectorCount;
        void *buffer;
        OSMessageQueue *queue;
    };

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    u32 sectorSize() override;
    bool read(u32 firstSector, u32 sectorCount, void *buffer) override;
    bool write(u32 firstSector, u32 sectorCount, const void *buffer) override;
    bool erase(u32 firstSector, u32 sectorCount) override;
    bool sync() override;

    void *run();
    void *transfer();
    void handleEXT();
    bool attach();
    void detach();
    void pollAdd();
    void pollRemove();

    bool dispatch(TransferFunc func, u32 firstSector, u32 sectorCount, void *buffer);
    bool dispatch(Transfer *transfer);
    bool execute(const Transfer *transfer);
    bool transferRead(u32 firstSector, u32 sectorCount, void *buffer);
    bool transferWrite(u32 firstSector, u32 sectorCount, void *buffer);
    bool transferErase(u32 firstSector, u32 sectorCount, void *buffer);

    bool sendCommandAndRecvR1(u8 command, u32 argument, u8 r1Mask = ~0);
    bool sendCommandAndRecvR1(EXI::Device &device, u8 command, u32 argument, u8 r1Mask = ~0);
    bool sendCommand(EXI::Device &device, u8 command, u32 argument);
    bool recvR1(EXI::Device &device, u8 &r1);
    bool recvR3(EXI::Device &device, u8 &r1, u32 &ocr);
    bool recvR7(EXI::Device &device, u8 &r1, u8 &commandVersion, u8 &vhs, u8 &checkPattern);
    bool waitReady(EXI::Device &device);

    static void *Run(void *param);
    static void *Transfer(void *param);
    static void HandleEXT(s32 chan, struct OSContext *context);
    static u8 ComputeCRC7(const u8 *buffer, u32 size);
    static u16 ComputeCRC16(const u8 *buffer, u32 size);

    u32 m_channel;
    bool m_wasDetached;
    bool m_isSDHC;
    struct OSMessageQueue *m_queue;
    struct OSMessageQueue *m_transferQueue;

    static Array<EXISDStorage *, 3> s_instances;
};
