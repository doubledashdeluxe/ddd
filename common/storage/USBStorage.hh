#pragma once

#include "common/USB.hh"
#include "common/storage/FATStorage.hh"

#ifdef PAYLOAD
#include <payload/Mutex.hh>
#endif

class USBStorage : private USB::Handler, private FATStorage {
public:
    static void Init();

private:
    class MSCRequest {
    public:
        enum {
            GetMaxLUN = 0xfe,
        };

    private:
        MSCRequest();
    };

    class SCSICommand {
    public:
        enum {
            TestUnitReady = 0x0,
            RequestSense = 0x3,
            Inquiry = 0x12,
            ReadCapacity10 = 0x25,
            Read10 = 0x28,
            Write10 = 0x2a,
            SynchronizeCache10 = 0x35,
        };

    private:
        SCSICommand();
    };

    class SCSIDeviceType {
    public:
        enum {
            DirectAccess = 0x0,
        };

    private:
        SCSIDeviceType();
    };

    USBStorage();
    ~USBStorage();

    void onRemove(USB::Device *device) override;
    bool onAdd(const USB::DeviceInfo *deviceInfo, USB::Device *device) override;
    void notify() override;

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    u32 sectorSize() override;
    bool read(u32 firstSector, u32 sectorCount, void *buffer) override;
    bool write(u32 firstSector, u32 sectorCount, const void *buffer) override;
    bool erase(u32 firstSector, u32 sectorCount) override;
    bool sync() override;

    bool testUnitReady(USB::Device *device, u8 lun);
    bool inquiry(USB::Device *device, u8 lun, u8 &type);
    bool initLun(USB::Device *device, u8 lun);
    bool requestSense(USB::Device *device, u8 lun);
    bool findLun(USB::Device *device, u8 lunCount, u8 &lun);
    bool readCapacity(USB::Device *device, u8 lun, u32 &blockSize);

    bool getLunCount(USB::Device *device, u8 &lunCount);
    bool scsiTransfer(USB::Device *device, bool isWrite, u32 size, void *data, u8 lun, u8 cbSize,
            void *cb);

    USB::Device *m_device;
    u8 m_interface;
    u8 m_outEndpointNumber;
    u8 m_inEndpointNumber;
    u32 m_tag;
    u8 m_lun;
    u32 m_blockSize;
    alignas(0x20) Array<u8, 0x1f> m_csw;
    alignas(0x20) Array<u8, 0xd> m_cbw;
    alignas(0x20) Array<u8, 0x4000> m_buffer;
#ifdef PAYLOAD
    Mutex m_mutex;
#endif

    static USBStorage *s_instance;
};
