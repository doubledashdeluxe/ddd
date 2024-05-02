#include "SDStorage.hh"

#include "common/Algorithm.hh"
#include "common/Log.hh"
#include "common/Memory.hh"

extern "C" {
#include <assert.h>
#include <string.h>
}

SDStorage::CardHandle::CardHandle(SDStorage *storage) : m_storage(storage) {
    if (!m_storage->sendCommand(Command::Select, 3, ResponseType::R1B, m_storage->m_rca << 16, 0, 0,
                nullptr, nullptr)) {
        m_storage = nullptr;
    }
}

SDStorage::CardHandle::~CardHandle() {
    if (m_storage) {
        m_storage->sendCommand(Command::Select, 3, ResponseType::R1B, 0, 0, 0, nullptr, nullptr);
    }
}

bool SDStorage::CardHandle::ok() const {
    return m_storage;
}

u32 SDStorage::priority() {
    return 1;
}

const char *SDStorage::prefix() {
    return "sd:";
}

u32 SDStorage::sectorSize() {
    return SectorSize;
}

bool SDStorage::read(u32 firstSector, u32 sectorCount, void *buffer) {
    return transfer(false, firstSector, sectorCount, buffer);
}

bool SDStorage::write(u32 firstSector, u32 sectorCount, const void *buffer) {
    return transfer(true, firstSector, sectorCount, const_cast<void *>(buffer));
}

bool SDStorage::erase(u32 /* firstSector */, u32 /* sectorCount */) {
    // TODO implement?
    return true;
}

bool SDStorage::sync() {
    // TODO implement?
    return true;
}

bool SDStorage::transfer(bool isWrite, u32 firstSector, u32 sectorCount, void *buffer) {
    assert(buffer);

    CardHandle cardHandle(this);
    if (!cardHandle.ok()) {
        return false;
    }

    u32 command = isWrite ? Command::WriteMultipleBlock : Command::ReadMultipleBlock;
    if (Memory::IsAligned(buffer, 0x20)) {
        u32 firstBlock = m_isSDHC ? firstSector : firstSector * SectorSize;
        return sendCommand(command, 3, ResponseType::R1, firstBlock, sectorCount, SectorSize,
                buffer, nullptr);
    } else {
        while (sectorCount > 0) {
            u32 chunkSectorCount = Min<u32>(sectorCount, m_buffer.count() / SectorSize);
            if (isWrite) {
                memcpy(m_buffer.values(), buffer, chunkSectorCount * SectorSize);
            }
            u32 firstBlock = m_isSDHC ? firstSector : firstSector * SectorSize;
            if (!sendCommand(command, 3, ResponseType::R1, firstBlock, chunkSectorCount, SectorSize,
                        m_buffer.values(), nullptr)) {
                return false;
            }
            if (!isWrite) {
                memcpy(buffer, m_buffer.values(), chunkSectorCount * SectorSize);
            }
            firstSector += chunkSectorCount;
            sectorCount -= chunkSectorCount;
            buffer = reinterpret_cast<u8 *>(buffer) + chunkSectorCount * SectorSize;
        }
        return true;
    }
}

bool SDStorage::enable4BitBus() {
    HostControl1 hostControl1;
    if (!readHCR(HCR::HostControl1, sizeof(hostControl1), &hostControl1)) {
        return false;
    }

    hostControl1.dataTransferWidth = 1;

    return writeHCR(HCR::HostControl1, sizeof(hostControl1), &hostControl1);
}

bool SDStorage::setCardBlockLength(u32 blockLength) {
    return sendCommand(Command::SetBlocklen, 3, ResponseType::R1, blockLength, 0, 0, nullptr,
            nullptr);
}

bool SDStorage::enableCard4BitBus() {
    if (!sendCommand(Command::AppCmd, 3, ResponseType::R1, m_rca << 16, 0, 0, nullptr, nullptr)) {
        return false;
    }

    return sendCommand(AppCommand::SetBusWidth, 3, ResponseType::R1, 0x2, 0, 0, nullptr, nullptr);
}

bool SDStorage::writeHCR(u8 reg, u8 size, const void *val) {
    assert(size <= 0x4);
    assert(val);

    alignas(0x20) RegOp regOp;
    memset(&regOp, 0, sizeof(regOp));
    regOp.reg = reg;
    regOp.size = size;
    memcpy(regOp.val + 0x4 - size, val, size);

    if (ioctl(Ioctl::WriteHCR, &regOp, sizeof(regOp), nullptr, 0) < 0) {
        DEBUG("Failed to write to host controller register 0x%x", reg);
        return false;
    }

    return true;
}

bool SDStorage::readHCR(u8 reg, u8 size, void *val) {
    assert(size <= 0x4);
    assert(val);

    alignas(0x20) RegOp regOp;
    memset(&regOp, 0, sizeof(regOp));
    regOp.reg = reg;
    regOp.size = size;
    alignas(0x20) u8 out[4];

    if (ioctl(Ioctl::ReadHCR, &regOp, sizeof(regOp), &out, sizeof(out)) < 0) {
        DEBUG("Failed to read host controller register 0x%x", reg);
        return false;
    }

    memcpy(val, out + 0x4 - size, size);
    return true;
}

bool SDStorage::resetCard() {
    alignas(0x20) u32 out;

    if (ioctl(Ioctl::ResetCard, nullptr, 0, &out, sizeof(out)) < 0) {
        DEBUG("Failed to reset interface");
        return false;
    }

    DEBUG("Successfully reset interface");
    m_rca = out >> 16;
    return true;
}

bool SDStorage::setClock(u32 clock) {
    alignas(0x20) u32 in = clock;

    if (ioctl(Ioctl::SetClock, &in, sizeof(in), nullptr, 0) < 0) {
        DEBUG("Failed to set clock");
        return false;
    }

    return true;
}

bool SDStorage::sendCommand(u32 command, u32 commandType, u32 responseType, u32 arg, u32 blockCount,
        u32 blockSize, void *buffer, u32 *response) {
    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = command;
    request.commandType = commandType;
    request.responseType = responseType;
    request.arg = arg;
    request.blockCount = blockCount;
    request.blockSize = blockSize;
    request.buffer = buffer;
    request.isDma = !!buffer;
    alignas(0x20) u32 out[4];

    if (command != VirtualCommand::Wait && command != VirtualCommand::Cancel &&
            (buffer || m_isSDHC)) {
        alignas(0x20) IoctlvPair pairs[3];
        pairs[0].data = &request;
        pairs[0].size = sizeof(request);
        pairs[1].data = buffer;
        pairs[1].size = blockCount * blockSize;
        pairs[2].data = out;
        pairs[2].size = sizeof(out);
        if (ioctlv(Ioctlv::SendCommand, 2, 1, pairs) < 0) {
            if (command != Command::Select) {
                DEBUG("Failed to send command 0x%x", command);
            }
            return false;
        }
    } else {
        if (ioctl(Ioctl::SendCommand, &request, sizeof(request), &out, sizeof(out)) < 0) {
            if (command != Command::Select) {
                DEBUG("Failed to send command 0x%x", command);
            }
            return false;
        }
    }

    if (response) {
        *response = out[0];
    }
    return true;
}

bool SDStorage::getStatus(Status &status) {
    alignas(0x20) u32 out;

    if (ioctl(Ioctl::GetStatus, nullptr, 0, &out, sizeof(out)) < 0) {
        DEBUG("Failed to get status");
        return false;
    }

    DEBUG("Got status 0x%x", out);
    memcpy(&status, &out, sizeof(out));
    return true;
}

const u32 SDStorage::SectorSize = 512;

SDStorage *SDStorage::s_instance = nullptr;
