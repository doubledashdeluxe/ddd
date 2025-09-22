#pragma once

extern "C" {
#include <dolphin/ETH.h>
#include <dolphin/OSContext.h>
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <portable/Array.hh>

class VirtualETH {
public:
    s32 init(s32 mode);
    void getMACAddr(u8 macaddr[6]);
    void setRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1);
    BOOL getLinkStateAsync(BOOL *status);
    void setProtoType(u16 *array, s32 num);
    void sendAsync(void *addr, s32 length, ETHCallback2 callback2);
    void addMulticastAddress(const u8 macaddr[6]);
    void clearMulticastAddresses();

    static void Init();
    static VirtualETH *Instance();

private:
    enum {
        BufferSize = 8192,
        TXStart = BufferSize - 1530,
        TXEnd = BufferSize - 1,
        RXStart = 0, // Must be 0 per erratum 5
        RXEnd = TXStart - 1,
    };

    enum {
        ERDPT = 0x00,    // Read Pointer
        EWRPT = 0x02,    // Write Pointer
        ETXST = 0x04,    // TX Start
        ETXND = 0x06,    // TX End
        ERXST = 0x08,    // RX Start
        ERXND = 0x0a,    // RX End
        ERXRDPT = 0x0c,  // RX RD Pointer
        ERXWRPT = 0x0e,  // RX WR Pointer
        EIE = 0x1b,      // Ethernet Interrupt Enable
        EIR = 0x1c,      // Ethernet Interrupt Request
        ESTAT = 0x1d,    // Ethernet Status
        ECON2 = 0x1e,    // Ethernet Control 2
        ECON1 = 0x1f,    // Ethernet Control 1
        ERXFCON = 0x38,  // Ethernet Receive Filter Control
        EPKTCNT = 0x39,  // Ethernet Packet Count
        MACON1 = 0x40,   // MAC Control 1
        MACON3 = 0x42,   // MAC Control 3
        MACON4 = 0x43,   // MAC Control 4
        MABBIPG = 0x44,  // MAC Back-to-Back Inter-Packet Gap
        MAIPG = 0x46,    // MAC Non-Back-to-Back Inter-Packet Gap
        MICMD = 0x52,    // MII Command
        MIREGADR = 0x54, // MII Register Addres
        MIWR = 0x56,     // MII Write Data
        MIRD = 0x58,     // MII Read Data
        MAADR5 = 0x60,   // MAC Address Byte 5
        MAADR6 = 0x61,   // MAC Address Byte 6
        MAADR3 = 0x62,   // MAC Address Byte 3
        MAADR4 = 0x63,   // MAC Address Byte 4
        MAADR1 = 0x64,   // MAC Address Byte 1
        MAADR2 = 0x65,   // MAC Address Byte 2
        MISTAT = 0x6a,   // MII Status
    };

    enum {
        PHCON1 = 0x00,  // PHY Control 1
        PHSTAT1 = 0x01, // Physical Layer Status 1
        PHID1 = 0x02,   // PHY Device ID 1
        PHID2 = 0x03,   // PHY Device ID 2
        PHCON2 = 0x10,  // PHY Control 2
        PHSTAT2 = 0x11, // Physical Layer Status 2
        PHIE = 0x12,    // PHY Interrupt Enable
        PHIR = 0x13,    // PHY Interrupt Request
        PHLCON = 0x14,  // PHY Module LED Control
    };

    struct RecvContext {
        ETHCallback0 callback0;
        ETHCallback1 callback1;
    };

    struct SendContext {
        void *addr;
        s32 length;
        ETHCallback2 callback2;
    };

    VirtualETH();

    void *run();
    bool attach();
    void detach();
    bool handleInterrupt();
    bool handlePacket();
    bool handleLinkChange();
    bool handleTransmit();
    bool handleTransmitError();
    bool trySend();
    void handleEXT();
    void handleEXI();
    bool init();
    bool initBuffer();
    bool initFilters();
    bool initMAC();
    bool initMACAddr();
    bool initPHY();
    bool initEthernet();

    bool readID(u32 &id);
    void reset();
    bool bitFieldSet(u8 address, u8 bits);
    bool bitFieldClear(u8 address, u8 bits);
    bool setBank(u8 bank);
    bool readControlRegister(u8 address, u8 &data, bool hasDummy);
    bool writeControlRegister(u8 address, u8 data);
    bool readControlRegister(u8 address, u16 &data, bool hasDummy);
    bool writeControlRegister(u8 address, u16 data);
    bool readPHYRegister(u8 address, u16 &data);
    bool writePHYRegister(u8 address, u16 data);
    bool readBufferMemory(void *buffer, u32 size);
    bool writeBufferMemory(const void *buffer, u32 size);

    bool read(u8 command, void *buffer, u32 size, u32 frequency = 4);
    bool write(u8 command, const void *buffer, u32 size, u32 frequency = 4);

    static void *Run(void *param);
    static void HandleEXT(s32 chan, OSContext *context);
    static void HandleEXI(s32 chan, OSContext *context);
    static void HandleEXI(s16 interrupt, OSContext *context);

    u32 m_channel;
    u32 m_device;
    bool m_wasDetached;
    u8 m_bank;
    bool m_latchingLinkStatus;
    bool m_linkStatus;
    u8 m_macAddr[6];
    RecvContext m_recvContext;
    SendContext m_sendContext;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;
    Array<u8, 4 * 1024> m_stack;
    OSThread m_thread;

    static VirtualETH *s_instance;
};
