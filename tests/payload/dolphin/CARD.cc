extern "C" {
#include <dolphin/CARD.h>
}
#include <payload/VirtualCard.hh>
#define SNITCH_IMPLEMENTATION
#include <snitch/snitch_all.hpp>

static u32 config;
static u32 state;

s32 REPLACED(CARDCheck)(s32 chan) {
    state |= 1 << chan;
    return config >> chan & 1 ? CARD_RESULT_READY : CARD_RESULT_NOCARD;
}

s32 REPLACED(CARDProbeEx)(s32 chan, s32 * /* memSize */, s32 * /* sectorSize */) {
    state |= 1 << chan;
    return config >> chan & 1 ? CARD_RESULT_READY : CARD_RESULT_NOCARD;
}

s32 REPLACED(CARDMount)(s32 chan, void * /* workArea */, CARDCallback /* detachCallback */) {
    state |= 1 << chan;
    return config >> chan & 1 ? CARD_RESULT_READY : CARD_RESULT_NOCARD;
}

void VirtualCard::Init() {
    for (u32 chan = 0; chan < s_instances.count(); chan++) {
        s_instances[chan].reset(new VirtualCard(chan));
    }
}

s32 VirtualCard::check() {
    state |= 1 << (2 + m_chan);
    return CARD_RESULT_READY;
}

s32 VirtualCard::probeEx(s32 * /* memSize */, s32 * /* sectorSize */) {
    state |= 1 << (2 + m_chan);
    return CARD_RESULT_READY;
}

s32 VirtualCard::mount(void * /* workArea */, CARDCallback /* detachCallback */) {
    state |= 1 << (2 + m_chan);
    return CARD_RESULT_READY;
}

TEST_CASE("CARD") {
    VirtualCard::Init();

    auto probeEx = [&](u32 config) {
        state = 0;
        for (s32 chan = 0; chan < CARD_NUM_CHANS; chan++) {
            s32 memSize, sectorSize;
            CHECK(CARDProbeEx(chan, &memSize, &sectorSize) == CARD_RESULT_READY);
        }
        CHECK((state >> 0 & 1));
        CHECK((state >> 1 & 1));
        CHECK((state >> 2 & 1) != (config >> 0 & 1));
        CHECK((state >> 3 & 1) != (config >> 1 & 1));
    };

    auto mount = [&](u32 config) {
        state = 0;
        for (s32 chan = 0; chan < CARD_NUM_CHANS; chan++) {
            CHECK(CARDMount(chan, nullptr, nullptr) == CARD_RESULT_READY);
        }
        CHECK((state >> 0 & 1) == (config >> 0 & 1));
        CHECK((state >> 1 & 1) == (config >> 1 & 1));
        CHECK((state >> 2 & 1) != (config >> 0 & 1));
        CHECK((state >> 3 & 1) != (config >> 1 & 1));
    };

    auto check = [&](u32 config) {
        state = 0;
        for (s32 chan = 0; chan < CARD_NUM_CHANS; chan++) {
            CARDCheck(chan);
        }
        CHECK((state >> 0 & 1) == (config >> 0 & 1));
        CHECK((state >> 1 & 1) == (config >> 1 & 1));
        CHECK((state >> 2 & 1) != (config >> 0 & 1));
        CHECK((state >> 3 & 1) != (config >> 1 & 1));
    };

    for (u32 firstConfig = 0; firstConfig < 1 << 4; firstConfig++) {
        for (u32 secondConfig = 0; secondConfig < 1 << 4; secondConfig++) {
            config = firstConfig;
            probeEx(firstConfig);
            mount(firstConfig);
            check(firstConfig);
            config = secondConfig;
            probeEx(secondConfig);
            check(firstConfig);
            mount(secondConfig);
            check(secondConfig);
        }
    }
}
