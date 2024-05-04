#pragma once

#include "common/Array.hh"
#include "common/ios/Resource.hh"

class Mutex;
extern "C" {
struct OSMessageQueue;
}

class USB {
public:
    class Device;
    class Handle;

private:
    struct Transfer;
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
    size_assert(DeviceDescriptor, 0x14);

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
    size_assert(ConfigDescriptor, 0xc);

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
    size_assert(InterfaceDescriptor, 0xc);

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
    size_assert(EndpointDescriptor, 0x8);

    struct DeviceInfo {
        u32 id;
        u8 _04[0x14 - 0x04];
        DeviceDescriptor deviceDescriptor;
        ConfigDescriptor configDescriptor;
        InterfaceDescriptor interfaceDescriptor;
        EndpointDescriptor endpointDescriptors[16];
    };
    size_assert(DeviceInfo, 0xc0);

    class Handler {
    public:
        void poll();

        virtual void onRemove(Device *device) = 0;
        virtual bool onAdd(const DeviceInfo *deviceInfo, Device *device) = 0;
        virtual void notify() = 0;

    protected:
        Handler();
        ~Handler();

    private:
        void pollRemove();
        void pollAdd();
        bool isRemovalHandler() const;
        bool isAdditionHandler() const;

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
        friend class Handle;
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
    size_assert(Transfer, 0x40);

    class Resource : protected IOS::Resource {
    public:
        struct Buffer {
            u8 buffer[0x180];
        };

        struct DeviceEntry {
            u32 id;
            u16 vendorId;
            u16 productId;
            u8 _8[0xa - 0x8];
            u8 interfaceNumber;
            u8 alternateSettingCount;
        };
        size_assert(DeviceEntry, 0xc);

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
        Resource(const char *name, Buffer &buffer);
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

        u8 (&m_buffer)[0x180];
    };

    class VENResource : public Resource {
    public:
        VENResource(Buffer &buffer);
        ~VENResource();

        bool getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo) override;
        bool setAlternateSetting(u32 id, u8 alternateSetting) override;
        bool intrTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber) override;
        bool isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes, u8 packets,
                u8 endpointDirection, u8 endpointNumber) override;
        bool bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber) override;
    };

    class HIDResource : public Resource {
    public:
        HIDResource(Buffer &buffer);
        ~HIDResource();

        bool getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo) override;
        bool setAlternateSetting(u32 id, u8 alternateSetting) override;
        bool intrTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber) override;
        bool isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes, u8 packets,
                u8 endpointDirection, u8 endpointNumber) override;
        bool bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length, u8 endpointDirection,
                u8 endpointNumber) override;
    };

    struct Backend {
        Resource::Buffer buffer;
        Resource *resource;
        Array<Device, 0x20> devices;
        OSMessageQueue *initQueue;
    };

public:
    class Handle {
    public:
        Handle();
        ~Handle();

    private:
        VENResource m_venResource;
        HIDResource m_hidResource;
    };

private:
    USB();

    static void *Run(void *param);
    static void PrintDeviceEntries(u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void CheckDeviceEntries(u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void HandleRemovals(Backend *backend, u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void HandleAdditions(Backend *backend, u32 deviceEntryCount,
            const Array<Resource::DeviceEntry, 0x20> &deviceEntries);
    static void HandleRemoval();
    static void HandleAddition();

    static DeviceInfo *s_additionDeviceInfo;
    static Handler *s_headHandler;
    static Handler *s_removalHandler;
    static Handler *s_additionHandler;
    static Device *s_removalDevice;
    static Device *s_additionDevice;
    static Array<Backend *, 2> s_backends;
    static OSMessageQueue s_queue;
    static Mutex *s_mutex;
};
