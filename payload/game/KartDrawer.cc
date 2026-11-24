#include "KartDrawer.hh"

#include "game/J2DManager.hh"
#include "game/SequenceInfo.hh"

u8 KartDrawer::ghostAlpha() const {
    return m_ghostAlpha;
}

bool KartDrawer::isVisible() const {
    return !m_hiddenFlags;
}

RaceKartLight *KartDrawer::light(u32 index) const {
    return m_lights[index];
}

void KartDrawer::drawKart(u32 screenIndex, u32 stage) {
    REPLACED(drawKart)(screenIndex, convertStage(screenIndex, stage));
}

void KartDrawer::drawDriver(u32 screenIndex, u32 stage) {
    REPLACED(drawDriver)(screenIndex, convertStage(screenIndex, stage));
}

u32 KartDrawer::convertStage(u32 screenIndex, u32 stage) {
    if (SequenceInfo::Instance().isOnlineTA() && J2DManager::KartStatus(m_index) != screenIndex) {
        stage = stage == Stage::Ghost ? m_stage : Stage::Ghost;
    }
    return stage;
}
