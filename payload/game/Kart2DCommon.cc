#include "Kart2DCommon.hh"

#include "game/ResMgr.hh"

#include <payload/UTF8.hh>
#include <portable/Algorithm.hh>

extern "C" {
#include <string.h>
}

void Kart2DCommon::changeUnicodeTexture(const char *text, u32 count, J2DScreen &screen,
        const char *prefix, f32 *startX, f32 *endX) {
    u32 length = UTF8::Length(text);
    for (u32 i = 0; i < count; i++) {
        J2DPicture *picture = screen.search("%s%u", prefix, i)->downcast<J2DPicture>();
        picture->m_isVisible = i < length;
        if (i < length) {
            u32 c = length > count && i + 3 >= count ? '.' : UTF8::Next(text);
            picture->changeTexture(getUnicodeTexture(c), 0);
        }
        if (startX && i == 0) {
            *startX = picture->m_offset.x;
        }
        if (endX && i + 1 == Max<u32>(length, 1)) {
            *endX = picture->m_offset.x;
        }
    }
}

void Kart2DCommon::changeNumberTexture(u32 number, u32 maxDigits, J2DScreen &screen,
        const char *prefix) {
    u32 digits = CountDigits(number);
    if (digits > maxDigits) {
        number = 999999999;
        digits = maxDigits;
    }
    for (u32 i = 1; i <= maxDigits; i++) {
        for (u32 j = 0; j < i; j++) {
            J2DPicture *picture = screen.search("%s%u%u", prefix, i, j)->downcast<J2DPicture>();
            picture->m_isVisible = i == digits;
        }
    }
    for (u32 i = 0; i < digits; i++) {
        J2DPicture *picture = screen.search("%s%u%u", prefix, digits, i)->downcast<J2DPicture>();
        picture->changeTexture(getNumberTexture(number % 10), 0);
        number /= 10;
    }
}

ResTIMG *Kart2DCommon::getAsciiTexture(char c) {
    const char *path;
    switch (c) {
    case '\'':
        path = "timg/MarioFontApostrophe.bti";
        break;
    case '/':
        path = "timg/MarioFontSlash.bti";
        break;
    default:
        return REPLACED(getAsciiTexture(c));
    }
    return reinterpret_cast<ResTIMG *>(ResMgr::GetPtr(ResMgr::ArchiveID::Race2D, path));
}

ResTIMG *Kart2DCommon::getUnicodeTexture(u32 cp) {
    const char *path;
    switch (cp) {
    case ' ':
    case '!':
    case '\'':
    case '+':
    case '-':
    case '.':
    case '/':
    case '?':
        return getAsciiTexture(cp);
    case 0x00c0: // À
    case 0x00e0: // à
        path = "timg/MarioFont_A_grave.bti";
        break;
    case 0x00c1: // Á
    case 0x00e1: // á
        path = "timg/MarioFont_A_acute.bti";
        break;
    case 0x00c4: // Ä
    case 0x00e4: // ä
        path = "timg/MarioFont_A_diaeresis.bti";
        break;
    case 0x00c9: // É
    case 0x00e9: // é
        path = "timg/MarioFont_E_acute.bti";
        break;
    case 0x00cc: // Ì
    case 0x00ec: // ì
        path = "timg/MarioFont_I_grave.bti";
        break;
    case 0x00cd: // Í
    case 0x00ed: // í
        path = "timg/MarioFont_I_acute.bti";
        break;
    case 0x00d1: // Ñ
    case 0x00f1: // ñ
        path = "timg/MarioFont_N_tilde.bti";
        break;
    case 0x00d3: // Ó
    case 0x00f3: // ó
        path = "timg/MarioFont_O_acute.bti";
        break;
    case 0x00d6: // Ö
    case 0x00f6: // ö
        path = "timg/MarioFont_O_diaeresis.bti";
        break;
    case 0x00dc: // Ü
    case 0x00fc: // ü
        path = "timg/MarioFont_U_diaeresis.bti";
        break;
    case 0x3044: // い
        path = "timg/MarioFontH_i.bti";
        break;
    case 0x3046: // う
        path = "timg/MarioFontH_u.bti";
        break;
    case 0x304d: // き
        path = "timg/MarioFontH_ki.bti";
        break;
    case 0x304f: // く
        path = "timg/MarioFontH_ku.bti";
        break;
    case 0x305b: // せ
        path = "timg/MarioFontH_se.bti";
        break;
    case 0x305f: // た
        path = "timg/MarioFontH_ta.bti";
        break;
    case 0x3060: // だ
        path = "timg/MarioFontH_da.bti";
        break;
    case 0x3069: // ど
        path = "timg/MarioFontH_do.bti";
        break;
    case 0x3075: // ふ
        path = "timg/MarioFontH_fu.bti";
        break;
    case 0x3078: // へ
        path = "timg/MarioFontH_he.bti";
        break;
    case 0x307c: // ぼ
        path = "timg/MarioFontH_bo.bti";
        break;
    case 0x308d: // ろ
        path = "timg/MarioFontH_ro.bti";
        break;
    case 0x3093: // ん
        path = "timg/MarioFontH_n.bti";
        break;
    case 0x30a3: // ィ
        path = "timg/MarioFontK_xi.bti";
        break;
    case 0x30a4: // イ
        path = "timg/MarioFontK_i.bti";
        break;
    case 0x30aa: // オ
        path = "timg/MarioFontK_o.bti";
        break;
    case 0x30ab: // カ
        path = "timg/MarioFontK_ka.bti";
        break;
    case 0x30ad: // キ
        path = "timg/MarioFontK_ki.bti";
        break;
    case 0x30af: // ク
        path = "timg/MarioFontK_ku.bti";
        break;
    case 0x30b0: // グ
        path = "timg/MarioFontK_gu.bti";
        break;
    case 0x30b3: // コ
        path = "timg/MarioFontK_ko.bti";
        break;
    case 0x30b5: // サ
        path = "timg/MarioFontK_sa.bti";
        break;
    case 0x30b7: // シ
        path = "timg/MarioFontK_si.bti";
        break;
    case 0x30b8: // ジ
        path = "timg/MarioFontK_zi.bti";
        break;
    case 0x30c3: // ッ
        path = "timg/MarioFontK_xtu.bti";
        break;
    case 0x30c6: // テ
        path = "timg/MarioFontK_te.bti";
        break;
    case 0x30c7: // デ
        path = "timg/MarioFontK_de.bti";
        break;
    case 0x30c8: // ト
        path = "timg/MarioFontK_to.bti";
        break;
    case 0x30c9: // ド
        path = "timg/MarioFontK_do.bti";
        break;
    case 0x30ce: // ノ
        path = "timg/MarioFontK_no.bti";
        break;
    case 0x30d0: // バ
        path = "timg/MarioFontK_ba.bti";
        break;
    case 0x30d6: // ブ
        path = "timg/MarioFontK_bu.bti";
        break;
    case 0x30d7: // プ
        path = "timg/MarioFontK_pu.bti";
        break;
    case 0x30dc: // ボ
        path = "timg/MarioFontK_bo.bti";
        break;
    case 0x30de: // マ
        path = "timg/MarioFontK_ma.bti";
        break;
    case 0x30df: // ミ
        path = "timg/MarioFontK_mi.bti";
        break;
    case 0x30e0: // ム
        path = "timg/MarioFontK_mu.bti";
        break;
    case 0x30e3: // ャ
        path = "timg/MarioFontK_xya.bti";
        break;
    case 0x30e9: // ラ
        path = "timg/MarioFontK_ra.bti";
        break;
    case 0x30ea: // リ
        path = "timg/MarioFontK_ri.bti";
        break;
    case 0x30eb: // ル
        path = "timg/MarioFontK_ru.bti";
        break;
    case 0x30ed: // ロ
        path = "timg/MarioFontK_ro.bti";
        break;
    case 0x30f3: // ン
        path = "timg/MarioFontK_n.bti";
        break;
    case 0x30fc: // ー
        path = "timg/MarioFontKH_psm.bti";
        break;
    default:
        if (cp >= '0' && cp <= '9') {
            return getAsciiTexture(cp);
        }
        if (cp >= 'A' && cp <= 'Z') {
            return getAsciiTexture(cp);
        }
        if (cp >= 'a' && cp <= 'z') {
            return getAsciiTexture(cp);
        }
        path = "timg/MarioFontQuestion.bti";
        break;
    }
    return reinterpret_cast<ResTIMG *>(ResMgr::GetPtr(ResMgr::ArchiveID::Race2D, path));
}

ResTIMG *Kart2DCommon::getNumberTexture(u32 index) {
    return m_numberTextures[index];
}

ResTIMG *Kart2DCommon::getCharacterIcon(u32 index) {
    return m_characterIcons[index];
}

ResTIMG *Kart2DCommon::getBattleIcon(u32 index) {
    return m_battleIcons[index];
}

Kart2DCommon *Kart2DCommon::Create(JKRHeap *heap) {
    if (!s_instance) {
        s_instance = new (heap, 0x4) Kart2DCommon;
    }
    return s_instance;
}

u32 Kart2DCommon::CountDigits(u32 number) {
    u32 digits = 0;
    do {
        digits++;
        number /= 10;
    } while (number > 0);
    return digits;
}
