#include "Course.hh"

#include "game/CourseID.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/SoundID.hh"

u32 Course::getCourseBGM() const {
    switch (ResMgr::GetMusicID()) {
    case CourseID::BabyLuigi:
        return SoundID::JA_STRM_BABYLUIGI;
    case CourseID::Peach:
        return SoundID::JA_STRM_PEACH;
    case CourseID::Daisy:
        return SoundID::JA_STRM_DAISY;
    case CourseID::Luigi:
        return SoundID::JA_STRM_LUIGI;
    case CourseID::Mario:
        return SoundID::JA_STRM_MARIO;
    case CourseID::Yoshi:
        return SoundID::JA_STRM_YOSI;
    case CourseID::Nokonoko:
        return SoundID::JA_STRM_NOKONOKO;
    case CourseID::Patapata:
        return SoundID::JA_STRM_PATAPATA;
    case CourseID::Waluigi:
        return SoundID::JA_STRM_WALUIGI;
    case CourseID::Wario:
        return SoundID::JA_STRM_WARIO;
    case CourseID::Diddy:
        return SoundID::JA_STRM_DIDDY;
    case CourseID::Donkey:
        return SoundID::JA_STRM_DONKEY;
    case CourseID::Koopa:
        return SoundID::JA_STRM_KOOPA;
    case 0x30:
    case CourseID::Rainbow:
        return SoundID::JA_STRM_RAINBOW;
    case CourseID::Desert:
        return SoundID::JA_STRM_DESERT;
    case CourseID::Snow:
        return SoundID::JA_STRM_SNOW;
    case CourseID::Mini1:
        if (RaceInfo::Instance().getRaceMode() == RaceMode::Escape) {
            return SoundID::JA_STRM_BATTLE2;
        } else {
            return SoundID::JA_STRM_KOOPA;
        }
    case CourseID::Mini2:
    case CourseID::Mini3:
    case 0x37:
    case 0x39:
    case CourseID::Mini7:
    case CourseID::Mini8:
        if (RaceInfo::Instance().getRaceMode() == RaceMode::Escape) {
            return SoundID::JA_STRM_BATTLE2;
        } else {
            return SoundID::JA_STRM_BATTLE;
        }
    case CourseID::Mini5:
        if (RaceInfo::Instance().getRaceMode() == RaceMode::Escape) {
            return SoundID::JA_STRM_BATTLE2;
        } else {
            return SoundID::JA_STRM_GURAGURA_MARIO;
        }
    case CourseID::Award:
        if (RaceInfo::Instance().getAwardKartNo() < 0) {
            return SoundID::JA_STRM_COMMENDATION2;
        } else {
            return SoundID::JA_STRM_COMMENDATION;
        }
    case CourseID::Ending:
        if (KartLocale::GetVideoFrameMode() == KartLocale::VideoFrameMode::FiftyHz) {
            return SoundID::JA_STRM_ENDING_PAL50;
        } else {
            return SoundID::JA_STRM_ENDING;
        }
    default:
        return SoundID::JA_STRM_MARIO;
    }
}

u32 Course::getFinalLapBGM() const {
    switch (ResMgr::GetMusicID()) {
    case CourseID::BabyLuigi:
        return SoundID::JA_STRM_FINALLAP_BABYLUIGI;
    case CourseID::Peach:
        return SoundID::JA_STRM_FINALLAP_PEACH;
    case CourseID::Daisy:
        return SoundID::JA_STRM_FINALLAP_DAISY;
    case CourseID::Luigi:
        return SoundID::JA_STRM_FINALLAP_LUIGI;
    case CourseID::Mario:
        return SoundID::JA_STRM_FINALLAP_MARIO;
    case CourseID::Yoshi:
        return SoundID::JA_STRM_FINALLAP_YOSI;
    case CourseID::Nokonoko:
        return SoundID::JA_STRM_FINALLAP_NOKONOKO;
    case CourseID::Patapata:
        return SoundID::JA_STRM_FINALLAP_PATAPATA;
    case CourseID::Waluigi:
        return SoundID::JA_STRM_FINALLAP_WALUIGI;
    case CourseID::Wario:
        return SoundID::JA_STRM_FINALLAP_WARIO;
    case CourseID::Diddy:
        return SoundID::JA_STRM_FINALLAP_DIDDY;
    case CourseID::Donkey:
        return SoundID::JA_STRM_FINALLAP_DONKEY;
    case CourseID::Koopa:
        return SoundID::JA_STRM_FINALLAP_KOOPA;
    case 0x30:
    case CourseID::Rainbow:
        return SoundID::JA_STRM_FINALLAP_RAINBOW;
    case CourseID::Desert:
        return SoundID::JA_STRM_FINALLAP_DESERT;
    case CourseID::Snow:
        return SoundID::JA_STRM_FINALLAP_SNOW;
    default:
        return SoundID::JA_STRM_FINALLAP_MARIO;
    }
}
