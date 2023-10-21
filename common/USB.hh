#pragma once

#include "common/Array.hh"
#include "common/ios/Resource.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Mutex.hh>
#endif

class USB {
public:
    class Device;
    struct Transfer;

private:
    class Resource;

public:
    class InterfaceClass {
    public:
        enum {
            MassStorage = 0x8,
        };

    private:
        InterfaceClass();
    };

    class InterfaceSubClass {
    public:
        enum {
            MassStorageSCSI = 0x6,
        };

    private:
        InterfaceSubClass();
    };

    class InterfaceProtocol {
    public:
        enum {
            MassStorageBulkOnly = 0x50,
        };

    private:
        InterfaceProtocol();
    };

    class EndpointDirection {
    public:
        enum {
            HostToDevice = 0x0,
            DeviceToHost = 0x1,
        };

    private:
        EndpointDirection();
    };

    class EndpointTransferType {
    public:
        enum {
            Control = 0x0,
            Isochronous = 0x1,
            Bulk = 0x2,
            Interrupt = 0x3,
        };

    private:
        EndpointTransferType();
    };

    class RequestType {
    public:
        enum {
            Standard = 0x0,
            Class = 0x1,
            Vendor = 0x2,
            Reserved = 0x3,
        };

    private:
        RequestType();
    };

    class RequestRecipient {
    public:
        enum {
            Device = 0x0,
            Interface = 0x1,
            Endpoint = 0x2,
            Other = 0x3,
        };

    private:
        RequestRecipient();
    };

    struct DeviceDescriptor {
        u8 length;
        u8 type;
        u16 usbVersion;
        u8 deviceClass;
        u8 deviceSubClass;
        u8 deviceProtocol;
        u8 maxPacketSize0;
        u16 vendorId;
        u16 productId;
        u16 deviceVersion;
        u8 manufacturerIndex;
        u8 productIndex;
        u8 serialNumberIndex;
        u8 numConfigurations;
        u8 _12[0x14 - 0x12];
    };
    static_assert(sizeof(DeviceDescriptor) == 0x14);

    struct ConfigDescriptor {
        u8 length;
        u8 type;
        u16 totalLength;
        u8 numInterfaces;
        u8 configurationValue;
        u8 configurationIndex;
        u8 attributes;
        u8 maxPower;
        u8 _9[0xc - 0x9];
    };
    static_assert(sizeof(ConfigDescriptor) == 0xc);

    struct InterfaceDescriptor {
        u8 length;
        u8 type;
        u8 interfaceNumber;
        u8 alternateSetting;
        u8 numEndpoints;
        u8 interfaceClass;
        u8 interfaceSubClass;
        u8 interfaceProtocol;
        u8 interfaceIndex;
        u8 _9[0xc - 0x9];
    };
    static_assert(sizeof(InterfaceDescriptor) == 0xc);

    struct EndpointDescriptor {
        u8 length;
        u8 type;
        struct {
            u8 direction : 1;
            u8 : 3;
            u8 number : 4;
        } endpointAddress;
        struct {
            u8 : 6;
            u8 transferType : 2;
        } attributes;
        u16 maxPacketSize;
        u8 interval;
        u8 _7[0x8 - 0x7];
    };
    static_assert(sizeof(EndpointDescriptor) == 0x8);

    struct DeviceInfo {
        u32 id;
        u8 _04[0x14 - 0x04];
        DeviceDescriptor deviceDescriptor;
        ConfigDescriptor configDescriptor;
        InterfaceDescriptor interfaceDescriptor;
        EndpointDescriptor endpointDescriptors[16];
    };
    static_assert(sizeof(DeviceInfo) == 0xc0);

    class Handler {
    public:
        Handler();

        void poll();

        virtual void onRemove(Device *device) = 0;
        virtual bool onAdd(const DeviceInfo *deviceInfo, Device *device) = 0;
        virtual void notify() = 0;

    private:
        void pollRemove();
        void pollAdd();

        ~Handler();

        Handler *m_next;
    };

    class Device {
    public:
        Device();
        ~Device();

        bool ctrlTransfer(u8 endpointDirection, u8 requestType, u8 requestRecipient, u8 request,
                u16 value, u16 index, u16 length, void *data);
        bool intrTransfer(void *data, u16 length, u8 endpointDirection, u8 endpointNumber);
        bool isoTransfer(void *data, u16 *packetSizes, u8 packets, u8 endpointDirection,
                u8 endpointNumber);
        bool bulkTransfer(void *data, u16 length, u8 endpointDirection, u8 endpointNumber);

    private:
        Resource *m_resource;
        u32 m_id;
        Handler *m_handler;
        Transfer *m_transfer;

        friend class USB;
        friend class Handler;
    };

    static void Init();

private:
    struct Transfer {
        u32 id;
        u8 _04[0x08 - 0x04];
        union {
            struct {
                struct {
                    u8 endpointDirection : 1;
                    u8 type : 2;
                    u8 recipient : 5;
                } requestType;
                u8 request;
                u16 value;
                u16 index;
                u16 length;
                void *data;
            } ctrl;
            struct {
                void *data;
                u16 length;
                struct {
                    u8 direction : 1;
                    u8 : 3;
                    u8 number : 4;
                } endpoint;
            } venIntr;
            struct {
                u32 endpointDirection;
            } hidIntr;
            struct {
                void *data;
                u16 *packetSizes;
                u8 packets;
                struct {
                    u8 direction : 1;
                    u8 : 3;
                    u8 number : 4;
                } endpoint;
            } iso;
            struct {
                void *data;
                u16 length;
                u8 _0e[0x12 - 0x0e];
                struct {
                    u8 direction : 1;
                    u8 : 3;
                    u8 number : 4;
                } endpoint;
            } bulk;
        };
        u8 _14[0x20 - 0x14];
        IOS::Resource::IoctlvPair pairs[4];
    };
    static_assert(sizeof(Transfer) == 0x40);

    class Resource : protected IOS::Resource {
    public:
        struct DeviceEntry {
            u32 id;
            u16 vendorId;
            u16 productId;
            u8 _8[0xa - 0x8];
            u8 interfaceNumber;
            u8 alternateSettingCount;
        };
        static_assert(sizeof(DeviceEntry) == 0xc);

        bool getVersion(u32 &version);
        s32 getDeviceChange(Array<DeviceEntry, 0x20> &deviceEntries);
        virtual bool getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo) = 0;
        bool attachFinish();
        virtual bool setAlternateSetting(u32 id, u8 alternateSetting) = 0;
        bool suspend(u32 id);
        bool resume(u32 id);
        bool ctrlTransfer(u32 id, Transfer *transfer, u8 endpointDirection, u8 requestType,
                u8 requestRecipient, u8 request, u16 value, u16 index, u16 length, void *data);
        virtual bool intrTransfer(u32 id, Transfer *transfer, void *data, u16 length,
                u8 endpointDirection, u8 endpointNumber) = 0;
        virtual bool isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes,
                u8 packets, u8 endpointDirection, u8 endpointNumber) = 0;
        virtual bool bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length,
                u8 endpointDirection, u8 endpointNumber) = 0;

    protected:
        Resource(const char *name);
        ~Resource();

        class Ioctl {
        public:
            enum {
                GetVersion = 0x0,
                GetDeviceChange = 0x1,
                GetDeviceInfo = 0x3,
                AttachFinish = 0x6,
                SetAlternateSetting = 0x7,
                SuspendResume = 0x10,
            };

        private:
            Ioctl();
        };

        class Ioctlv {
        public:
            enum {
                CtrlTransfer = 0x12,
                IntrTransfer = 0x13,
                IsoTransfer = 0x14,
                BulkTransfer = 0x15,
            };

        private:
            Ioctlv();
        };

        alignas(0x20) u8 m_buffer[0x180];
    };

    class VENResource : public Resource {
    public:
        VENResource();
        ~VENResource();

        bool getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo);
        bool setAlternateSetting(u32 id, u8 alternateSetting);
        bool intrTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber);
        bool isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes, u8 packets,
                u8 endpointDirection, u8 endpointNumber);
        bool bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber);
    };

    class HIDResource : public Resource {
    public:
        HIDResource();
        ~HIDResource();

        bool getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo);
        bool setAlternateSetting(u32 id, u8 alternateSetting);
        bool intrTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber);
        bool isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes, u8 packets,
                u8 endpointDirection, u8 endpointNumber);
        bool bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber);
    };

    struct Backend {
        Array<Device, 0x20> devices;
        Resource *resource;
    };

    USB();

#ifdef PAYLOAD
    static void *Run(void *param);
#endif
    static void PrintDeviceEntries(u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void CheckDeviceEntries(u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void HandleRemovals(Backend *backend, u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void HandleAdditions(Backend *backend, u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);

    static DeviceInfo *s_additionDeviceInfo;
    static Handler *s_headHandler;
    static Handler *s_removalHandler;
    static Handler *s_additionHandler;
    static Device *s_removalDevice;
    static Device *s_additionDevice;
    static Backend *s_venBackend;
    static Backend *s_hidBackend;
#ifdef PAYLOAD
    static OSMessageQueue s_queue;
    static Array<OSMessage, 1> s_messages;
    static Mutex *s_mutex;
#endif
};