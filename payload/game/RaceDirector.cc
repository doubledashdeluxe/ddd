#include "RaceDirector.hh"

#include "game/PauseChoice.hh"
#include "game/PauseManager.hh"
#include "game/RaceClient.hh"
#include "game/RacePhase.hh"
#include "game/SequenceInfo.hh"

u32 RaceDirector::racePhase() const {
    return m_racePhase;
}

bool RaceDirector::isFrameRenewal() const {
    return m_isFrameRenewal;
}

bool RaceDirector::raceEnd() const {
    return m_raceEnd;
}

void RaceDirector::calc(s32 adjustment) {
    PauseManager *pauseManager = PauseManager::Instance();
    bool raceEnd = false;
    if (adjustment >= 0) {
        if (m_racePhase == RacePhase::Running) {
            raceEnd = checkRaceEnd();
            if (raceEnd) {
                pauseManager->m_pauseEnd = true;
            }
        }
    }
    if (adjustment <= 0) {
        pauseManager->exec();
    }
    if (adjustment >= 0) {
        if (pauseManager->paused()) {
            KartGamePad *pad = KartGamePad::GamePad(0);
            pad->expand(0);
        }
    }
    m_isFrameRenewal = true;
    switch (m_racePhase) {
    case RacePhase::CourseDemo:
        doCourseDemo();
        break;
    case RacePhase::NoCourseDemo:
        doNoCourseDemo();
        break;
    case RacePhase::Reset:
    case RacePhase::Restart:
    case RacePhase::Replay:
        doReset();
        break;
    case RacePhase::Safety:
        doSafety();
        break;
    case RacePhase::Running:
        if (adjustment >= 0) {
            doRunning(raceEnd);
        } else {
            checkPauseChoice();
        }
        break;
    case RacePhase::Wait:
        doWait();
        break;
    case RacePhase::Ending:
        doEnding();
        break;
    }
}

void RaceDirector::resetCommon() {
    m_race2DMode = Race2DMode::OneTwo;

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline && m_racePhase == RacePhase::CourseDemo) {
        m_racePhase = RacePhase::NoCourseDemo;
    }

    REPLACED(resetCommon)();
}

void RaceDirector::doRunning(bool r4) {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient && !raceClient->ok()) {
        setPhaseWait(RacePhase::Title, true, true, 35);
    }

    REPLACED(doRunning)(r4);
}

void RaceDirector::doEnding() {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient && m_endingState == 4 && m_frame == 59) {
        if (!raceClient->hasResults()) {
            return;
        }

        PauseManager::Instance()->startResult();
    }

    REPLACED(doEnding)();
}

void RaceDirector::checkPauseChoice() {
    switch (PauseManager::PauseChoice()) {
    case PauseChoice::PersonalRoom:
        setPhaseWait(RacePhase::PersonalRoom, true, true, 35);
        return;
    case PauseChoice::PlayerList:
        setPhaseWait(RacePhase::PlayerList, true, true, 35);
        return;
    }

    REPLACED(checkPauseChoice)();
}

bool RaceDirector::checkRaceEnd() const {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient) {
        return RaceClient::Frame() + 1 >= raceClient->endFrame();
    }

    return REPLACED(checkRaceEnd)();
}
