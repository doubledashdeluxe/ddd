#pragma once

#include "common/storage/FATStorage.hh"

class SDStorage : protected FATStorage {
protected:
    enum {
        SectorSize = 512,
    };

    class Command {
    public:
        enum {
            GoIdleState = 0,
            SendOpCond = 1,
            Select = 7,
            SendIfCond = 8,
            StopTransmission = 12,
            SetBlocklen = 16,
            ReadMultipleBlock = 18,
            WriteMultipleBlock = 25,
            EraseWrBlkStartAddr = 32,
            EraseWrBlkEndAddr = 33,
            Erase = 38,
            AppCmd = 55,
            ReadOCR = 58,
            CRCOnOff = 59,
        };

    private:
        Command();
    };

    class AppCommand {
    public:
        enum {
            SetBusWidth = 6,
            SDSendOpCond = 41,
        };

    private:
        AppCommand();
    };

    SDStorage(Mutex *mutex);
    ~SDStorage();
};
