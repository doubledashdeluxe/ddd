#include <common/DiscID.hh>
#include <helpers/FakeStorage.hh>
#include <payload/VirtualCard.hh>
#define SNITCH_IMPLEMENTATION
#include <snitch/snitch_all.hpp>

#include <cstring>

extern "C" {
DiscID discID;
u32 busClock = 243000000;
}

extern "C" s64 OSGetTime() {
    return 0;
}

extern "C" void CARDUpdateIconOffsets(void * /* block */, CARDStat * /* stat */) {}

TEST_CASE("VirtualCard") {
    memcpy(discID.gameID, "GM4P", strlen("GM4P"));
    memcpy(discID.makerID, "01", strlen("01"));

    VirtualCard::Init();
    FakeStorage fakeStorage;
    Storage::CreateDir("main:/ddd", Storage::Mode::WriteAlways);

    for (s32 chan = 0; chan < CARD_NUM_CHANS; chan++) {
        VirtualCard *virtualCard = VirtualCard::Instance(chan);
        s32 memSize, sectorSize;
        CHECK(virtualCard->probeEx(&memSize, &sectorSize) == CARD_RESULT_READY);
        CHECK(virtualCard->check() != CARD_RESULT_READY);
        s32 bytesNotUsed, filesNotUsed;
        CHECK(virtualCard->freeBlocks(&bytesNotUsed, &filesNotUsed) != CARD_RESULT_READY);
        CHECK(virtualCard->mount(nullptr, nullptr) == CARD_RESULT_READY);
        CHECK(virtualCard->check() == CARD_RESULT_READY);
        CHECK(virtualCard->freeBlocks(&bytesNotUsed, &filesNotUsed) == CARD_RESULT_READY);
        CARDStat stat;
        for (s32 fileNo = 0; fileNo < 127; fileNo++) {
            CHECK(virtualCard->getStatus(fileNo, &stat) == CARD_RESULT_NOFILE);
        }
        CARDFileInfo fileInfo;
        CHECK(virtualCard->create("MarioKart Double Dash!!", sectorSize * 5, &fileInfo) ==
                CARD_RESULT_READY);
        CHECK(fileInfo.chan == chan);
        CHECK(virtualCard->getStatus(fileInfo.fileNo, &stat) == CARD_RESULT_READY);
        std::vector<u8> values(sectorSize * 5);
        for (size_t i = 0; i < values.size(); i++) {
            values[i] = i;
        }
        CHECK(virtualCard->write(&fileInfo, values.data(), values.size(), 0) == CARD_RESULT_READY);
        CHECK(virtualCard->setStatus(fileInfo.fileNo, &stat) == CARD_RESULT_READY);
        CHECK(virtualCard->close(&fileInfo) == CARD_RESULT_READY);
        CHECK(virtualCard->open("MarioKart Double Dash!!", &fileInfo) == CARD_RESULT_READY);
        values.clear();
        values.resize(sectorSize * 5);
        CHECK(virtualCard->read(&fileInfo, values.data(), values.size(), 0) == CARD_RESULT_READY);
        for (size_t i = 0; i < values.size(); i++) {
            CHECK(values[i] == (i & 0xff));
        }
        CHECK(virtualCard->close(&fileInfo) == CARD_RESULT_READY);
        CHECK(virtualCard->unmount() == CARD_RESULT_READY);
    }
}
