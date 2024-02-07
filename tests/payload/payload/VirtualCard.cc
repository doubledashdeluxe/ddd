#include <helpers/FakeStorage.hh>
#include <lest.hpp>
#include <payload/VirtualCard.hh>

#include <cstring>

extern "C" {
u32 discID[8];
u32 busClock = 243000000;
}

extern "C" s64 OSGetTime() {
    return 0;
}

extern "C" void CARDUpdateIconOffsets(void * /* block */, CARDStat * /* stat */) {}

void VirtualCard::Init() {
    for (u32 chan = 0; chan < s_instances.count(); chan++) {
        s_instances[chan].reset(new VirtualCard(chan));
    }
}

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("VirtualCard") {
    memset(discID, 0, sizeof(discID));
    memcpy(discID, "GM4P01", strlen("GM4P01"));

    VirtualCard::Init();
    FakeStorage fakeStorage;
    Storage::CreateDir("main:/ddd", Storage::Mode::WriteAlways);

    for (s32 chan = 0; chan < CARD_NUM_CHANS; chan++) {
        VirtualCard *virtualCard = VirtualCard::Instance(chan);
        s32 memSize, sectorSize;
        EXPECT(virtualCard->probeEx(&memSize, &sectorSize) == CARD_RESULT_READY);
        EXPECT(virtualCard->check() != CARD_RESULT_READY);
        s32 bytesNotUsed, filesNotUsed;
        EXPECT(virtualCard->freeBlocks(&bytesNotUsed, &filesNotUsed) != CARD_RESULT_READY);
        EXPECT(virtualCard->mount(nullptr, nullptr) == CARD_RESULT_READY);
        EXPECT(virtualCard->check() == CARD_RESULT_READY);
        EXPECT(virtualCard->freeBlocks(&bytesNotUsed, &filesNotUsed) == CARD_RESULT_READY);
        CARDStat stat;
        for (s32 fileNo = 0; fileNo < 127; fileNo++) {
            EXPECT(virtualCard->getStatus(fileNo, &stat) == CARD_RESULT_NOFILE);
        }
        CARDFileInfo fileInfo;
        EXPECT(virtualCard->create("MarioKart Double Dash!!", sectorSize * 5, &fileInfo) ==
                CARD_RESULT_READY);
        EXPECT(fileInfo.chan == chan);
        EXPECT(virtualCard->getStatus(fileInfo.fileNo, &stat) == CARD_RESULT_READY);
        std::vector<u8> values(sectorSize * 5);
        for (size_t i = 0; i < values.size(); i++) {
            values[i] = i;
        }
        EXPECT(virtualCard->write(&fileInfo, values.data(), values.size(), 0) == CARD_RESULT_READY);
        EXPECT(virtualCard->setStatus(fileInfo.fileNo, &stat) == CARD_RESULT_READY);
        EXPECT(virtualCard->close(&fileInfo) == CARD_RESULT_READY);
        EXPECT(virtualCard->open("MarioKart Double Dash!!", &fileInfo) == CARD_RESULT_READY);
        values.clear();
        values.resize(sectorSize * 5);
        EXPECT(virtualCard->read(&fileInfo, values.data(), values.size(), 0) == CARD_RESULT_READY);
        for (size_t i = 0; i < values.size(); i++) {
            EXPECT(values[i] == (i & 0xff));
        }
        EXPECT(virtualCard->close(&fileInfo) == CARD_RESULT_READY);
        EXPECT(virtualCard->unmount() == CARD_RESULT_READY);
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
