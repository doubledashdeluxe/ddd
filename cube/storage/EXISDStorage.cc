// clang-format off
//
// Resources:
// - https://elm-chan.org/docs/mmc/mmc_e.html
//
// clang-format on

#include "EXISDStorage.hh"

#include "cube/Clock.hh"

#include <portable/Bytes.hh>
#include <portable/Log.hh>

u32 EXISDStorage::priority() {
    return 3 + m_index;
}

const char *EXISDStorage::prefix() {
    return "exisd:";
}

u32 EXISDStorage::sectorSize() {
    return SectorSize;
}

bool EXISDStorage::read(u32 firstSector, u32 sectorCount, void *buffer) {
    return dispatch(&EXISDStorage::transferRead, firstSector, sectorCount, buffer);
}

bool EXISDStorage::write(u32 firstSector, u32 sectorCount, const void *buffer) {
    return dispatch(&EXISDStorage::transferWrite, firstSector, sectorCount,
            const_cast<void *>(buffer));
}

bool EXISDStorage::erase(u32 firstSector, u32 sectorCount) {
    return dispatch(&EXISDStorage::transferErase, firstSector, sectorCount, nullptr);
}

bool EXISDStorage::sync() {
    return true;
}

void EXISDStorage::pollAdd() {
    u32 id;
    if (!EXI::GetID(m_channel, m_device, id)) {
        TRACE("Failed to get ID");
        return;
    }

    if (id != 0xffffffff) {
        TRACE("Not an SD card");
        return;
    }

    {
        EXI::Device device(m_channel, 32, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return;
        }
        for (u32 i = 0; i < 74; i++) {
            u8 dummy = 0xff;
            device.immWrite(&dummy, sizeof(dummy));
        }
    }

    u32 argument = 0;
    if (!sendCommandAndRecvR1(Command::GoIdleState, argument, ~(1 << 0))) {
        return;
    }

    bool isV2;
    {
        EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return;
        }

        u8 vhs = 1 << 0;
        u8 checkPattern = 0xaa;
        argument = vhs << 8 | checkPattern << 0;
        if (!sendCommand(device, Command::SendIfCond, argument)) {
            DEBUG("Failed to send CMD8");
            return;
        }

        u8 r1, commandVersion, vhsEcho, checkPatternEcho;
        if (!recvR7(device, r1, commandVersion, vhsEcho, checkPatternEcho)) {
            DEBUG("Failed to receive R7 for CMD8");
            return;
        }

        if (r1 & ~(1 << 2 | 1 << 0)) {
            DEBUG("CMD8 error");
            return;
        }

        isV2 = !(r1 & 1 << 2);
        if (isV2) {
            if (vhsEcho != vhs) {
                DEBUG("Mismatched VHS");
                return;
            }
            if (checkPatternEcho != checkPattern) {
                DEBUG("Mismatched check pattern");
                return;
            }
        }
    }

    argument = 1 << 0;
    u8 r1;
    if (!sendCommandAndRecvR1(Command::CRCOnOff, argument, ~(1 << 0))) {
        return;
    }

    s64 start = Clock::GetMonotonicTicks();
    bool isMMC = false;
    do {
        EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return;
        }

        if (isMMC) {
            argument = 1 << 30;
            if (!sendCommand(device, Command::SendOpCond, argument)) {
                DEBUG("Failed to send CMD1");
                return;
            }

            if (!recvR1(device, r1)) {
                DEBUG("Failed to receive R1 for CMD1");
                return;
            }

            if (r1 & ~(r1 << 0)) {
                DEBUG("CMD1 error");
                return;
            }
        } else {
            argument = 0;
            if (!sendCommand(device, Command::AppCmd, argument)) {
                DEBUG("Failed to send CMD55");
                return;
            }

            if (!recvR1(device, r1)) {
                DEBUG("Failed to receive R1 for CMD55");
                return;
            }

            if (r1 & ~(!isV2 << 2 | 1 << 0)) {
                DEBUG("CMD55 error");
                return;
            }

            if (r1 & 1 << 2) {
                isMMC = true;
                continue;
            }

            argument = 1 << 30;
            if (!sendCommand(device, AppCommand::SDSendOpCond, argument)) {
                DEBUG("Failed to send ACMD41");
                return;
            }

            if (!recvR1(device, r1)) {
                DEBUG("Failed to receive R1 for ACMD41");
                return;
            }

            if (r1 & ~(1 << 0)) {
                DEBUG("ACMD41 error");
                return;
            }
        }

        if (!(r1 & 1 << 0)) {
            break;
        }
    } while (Clock::GetMonotonicTicks() < start + Clock::SecondsToTicks(2));

    if (r1 & 1 << 0) {
        DEBUG("Timed out");
        return;
    }

    m_isSDHC = false;
    if (isV2) {
        EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return;
        }

        argument = 0;
        if (!sendCommand(device, Command::ReadOCR, argument)) {
            DEBUG("Failed to send CMD58");
            return;
        }

        u32 ocr;
        if (!recvR3(device, r1, ocr)) {
            DEBUG("Failed to receive R3 for CMD58");
            return;
        }

        if (r1) {
            DEBUG("CMD58 error");
            return;
        }

        m_isSDHC = ocr & 1 << 30;
    }
    if (!m_isSDHC) {
        argument = SectorSize;
        if (!sendCommandAndRecvR1(Command::SetBlocklen, SectorSize)) {
            return;
        }
    }

    add();
}

void EXISDStorage::pollRemove() {
    remove();
}

bool EXISDStorage::dispatch(TransferFunc func, u32 firstSector, u32 sectorCount, void *buffer) {
    struct Transfer transfer;
    transfer.func = func;
    transfer.firstSector = firstSector;
    transfer.sectorCount = sectorCount;
    transfer.buffer = buffer;
    return dispatch(&transfer);
}

bool EXISDStorage::execute(const struct Transfer *transfer) {
    TransferFunc func = transfer->func;
    u32 firstSector = transfer->firstSector;
    u32 sectorCount = transfer->sectorCount;
    void *buffer = transfer->buffer;
    return (this->*func)(firstSector, sectorCount, buffer);
}

bool EXISDStorage::transferRead(u32 firstSector, u32 sectorCount, void *buffer) {
    for (u32 sector = firstSector; sector < firstSector + sectorCount;
            sector++, buffer = static_cast<u8 *>(buffer) + SectorSize) {
        EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return false;
        }

        u32 block = m_isSDHC ? sector : sector * SectorSize;
        if (!sendCommandAndRecvR1(device, Command::ReadSingleBlock, block)) {
            return false;
        }

        u8 token;
        do {
            if (!device.immRead(&token, sizeof(token))) {
                DEBUG("Failed to read token");
                return false;
            }
        } while (token != 0xfe);

        if (!device.immRead(buffer, SectorSize)) {
            DEBUG("Failed to read sector");
            return false;
        }

        Array<u8, 2> crc16;
        if (!device.immRead(crc16.values(), crc16.count())) {
            DEBUG("Failed to read CRC16");
            return false;
        }

        if (ComputeCRC16(static_cast<u8 *>(buffer), SectorSize) !=
                Bytes::ReadBE<u16>(crc16.values(), 0)) {
            DEBUG("Mismatched CRC16");
            return false;
        }
    }

    return true;
}

bool EXISDStorage::transferWrite(u32 firstSector, u32 sectorCount, void *buffer) {
    for (u32 sector = firstSector; sector < firstSector + sectorCount;
            sector++, buffer = static_cast<u8 *>(buffer) + SectorSize) {
        EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
        if (!device.ok()) {
            DEBUG("Failed to select device");
            return false;
        }

        u32 block = m_isSDHC ? sector : sector * SectorSize;
        if (!sendCommandAndRecvR1(device, Command::WriteSingleBlock, block)) {
            return false;
        }

        u8 token = 0xfe;
        if (!device.immWrite(&token, sizeof(token))) {
            DEBUG("Failed to write token");
            return false;
        }

        if (!device.dmaWrite(buffer, SectorSize)) {
            DEBUG("Failed to write sector");
            return false;
        }

        Array<u8, 2> crc16;
        Bytes::WriteBE<u16>(crc16.values(), 0,
                ComputeCRC16(static_cast<const u8 *>(buffer), SectorSize));
        if (!device.immWrite(crc16.values(), crc16.count())) {
            DEBUG("Failed to write CRC16");
            return false;
        }

        if (!device.immRead(&token, sizeof(token))) {
            DEBUG("Failed to read token");
            return false;
        }

        if ((token & 0x1f) != 0x05) {
            DEBUG("Invalid token");
            return false;
        }

        if (!waitReady(device)) {
            return false;
        }
    }

    return true;
}

bool EXISDStorage::transferErase(u32 firstSector, u32 sectorCount, void * /* buffer */) {
    EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
    if (!device.ok()) {
        DEBUG("Failed to select device");
        return false;
    }

    u32 firstBlock = m_isSDHC ? firstSector : firstSector * SectorSize;
    if (!sendCommandAndRecvR1(device, Command::EraseWrBlkStartAddr, firstBlock)) {
        return false;
    }

    u32 lastBlock = firstBlock + (m_isSDHC ? sectorCount : sectorCount * SectorSize) - 1;
    if (!sendCommandAndRecvR1(device, Command::EraseWrBlkEndAddr, lastBlock)) {
        return false;
    }

    if (!sendCommandAndRecvR1(device, Command::Erase, 0)) {
        return false;
    }

    return waitReady(device);
}

bool EXISDStorage::sendCommandAndRecvR1(u8 command, u32 argument, u8 r1Mask) {
    EXI::Device device(m_channel, m_device, 5, &m_wasDetached);
    if (!device.ok()) {
        DEBUG("Failed to select device");
        return false;
    }

    return sendCommandAndRecvR1(device, command, argument, r1Mask);
}

bool EXISDStorage::sendCommandAndRecvR1(EXI::Device &device, u8 command, u32 argument, u8 r1Mask) {
    if (!sendCommand(device, command, argument)) {
        DEBUG("Failed to send CMD%u", command);
        return false;
    }

    u8 r1;
    if (!recvR1(device, r1)) {
        DEBUG("Failed to receive R1 for CMD%u", command);
        return false;
    }

    if (r1 & r1Mask) {
        DEBUG("CMD%u error %02x", command, r1);
        return false;
    }

    return true;
}

bool EXISDStorage::sendCommand(EXI::Device &device, u8 command, u32 argument) {
    Array<u8, 6> buffer;
    Bytes::WriteBE<u8>(buffer.values(), 0, 1 << 6 | command << 0);
    Bytes::WriteBE<u32>(buffer.values(), 1, argument);
    Bytes::WriteBE<u8>(buffer.values(), 5, ComputeCRC7(buffer.values(), 5) << 1 | 1 << 0);
    return device.immWrite(buffer.values(), buffer.count());
}

bool EXISDStorage::recvR1(EXI::Device &device, u8 &r1) {
    for (u32 i = 0; i < 8; i++) {
        Array<u8, 1> buffer;
        if (!device.immRead(buffer.values(), buffer.count())) {
            return false;
        }
        r1 = Bytes::ReadBE<u8>(buffer.values(), 0);
        if (!(r1 & 1 << 7)) {
            return true;
        }
    }
    return false;
}

bool EXISDStorage::recvR3(EXI::Device &device, u8 &r1, u32 &ocr) {
    if (!recvR1(device, r1)) {
        return false;
    }
    if (r1 & (1 << 3 | 1 << 2)) {
        return true;
    }
    Array<u8, 4> buffer;
    if (!device.immRead(buffer.values(), buffer.count())) {
        return false;
    }
    ocr = Bytes::ReadBE<u32>(buffer.values(), 0);
    return true;
}

bool EXISDStorage::recvR7(EXI::Device &device, u8 &r1, u8 &commandVersion, u8 &vhs,
        u8 &checkPattern) {
    if (!recvR1(device, r1)) {
        return false;
    }
    if (r1 & (1 << 3 | 1 << 2)) {
        return true;
    }
    Array<u8, 4> buffer;
    if (!device.immRead(buffer.values(), buffer.count())) {
        return false;
    }
    commandVersion = Bytes::ReadBE<u8>(buffer.values(), 0) >> 4 & 0xf;
    vhs = Bytes::ReadBE<u8>(buffer.values(), 2) >> 0 & 0xf;
    checkPattern = Bytes::ReadBE<u8>(buffer.values(), 3);
    return true;
}

bool EXISDStorage::waitReady(EXI::Device &device) {
    while (true) {
        u8 token;
        if (!device.immRead(&token, sizeof(token))) {
            DEBUG("Failed to read token");
            return false;
        }

        if (token == 0xff) {
            return true;
        }

        device.release();
        if (!device.acquire(m_channel, m_device, 5, &m_wasDetached)) {
            DEBUG("Failed to select device");
            return false;
        }
    }
}

u32 EXISDStorage::IndexToChannel(u32 index) {
    return index < 2 ? 0 : index - 1;
}

u32 EXISDStorage::IndexToDevice(u32 index) {
    return index == 1 ? 2 : 0;
}

u8 EXISDStorage::ComputeCRC7(const u8 *buffer, u32 size) {
    u8 crc7 = 0;
    for (u32 i = 0; i < size; i++) {
        crc7 ^= buffer[i];
        for (u32 j = 0; j < 8; j++) {
            u8 poly = crc7 >> 7 ? 0x09 << 1 : 0x0;
            crc7 <<= 1;
            crc7 ^= poly;
        }
    }
    return crc7 >> 1;
}

u16 EXISDStorage::ComputeCRC16(const u8 *buffer, u32 size) {
    u16 crc16 = 0;
    for (u32 i = 0; i < size; i++) {
        crc16 ^= buffer[i] << 8;
        for (u32 j = 0; j < 8; j++) {
            u16 poly = crc16 >> 15 ? 0x1021 : 0x0;
            crc16 <<= 1;
            crc16 ^= poly;
        }
    }
    return crc16;
}
