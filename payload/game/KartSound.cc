#include "KartSound.hh"

#include "game/OnlineInfo.hh"
#include "game/RaceClient.hh"

void KartSound::doHorn() {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient) {
        u32 index = m_body->getIndex();
        if (!OnlineInfo::Instance().m_karts[index].local && raceClient->itemEvent(index)) {
            return;
        }
    }

    REPLACED(doHorn)();
}
