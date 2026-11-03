#include "KartGamePad.hh"

static void ConvertBtn(u16 *output, u16 input, u16 inputMask, u16 outputMask) {
    *output |= (input & inputMask) ? outputMask : 0;
}

s32 KartGamePad::padPort() const {
    return m_padPort;
}

KartGamePad *KartGamePad::GamePad(u32 index) {
    return s_gamePads[index];
}

KartGamePad *KartGamePad::KartPad(u32 index) {
    return s_kartPads[index];
}

void KartGamePad::expand(const PadMgr::KartPadData &data) {
    PADStatus padStatus;
    padStatus.stickX = data.stickX * 4.0f;
    padStatus.stickY = data.stickY * 18.0f;
    padStatus.analogA = 0;
    padStatus.analogB = 0;
    padStatus.triggerLeft = 0;
    padStatus.triggerRight = 0;

    u16 mappedButtons = 0;
    // Existing mappings; those used for the in-game physics
    ConvertBtn(&mappedButtons, data.buttons, 1 << 0, PAD_BUTTON_A);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 1, PAD_BUTTON_B);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 2, PAD_BUTTON_X);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 3, PAD_BUTTON_Y);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 4, PAD_TRIGGER_L);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 5, PAD_TRIGGER_R);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 6, PAD_TRIGGER_Z);
    ConvertBtn(&mappedButtons, data.buttons, 1 << 7, PAD_BUTTON_START);

    // Added logic for D-Pad Inputs
    if (m_padPort >= 0 && m_padPort < 4) {
        // The 4 D-Pad inputs exist in the lowest 4 bits of button bitfield.
        mappedButtons |= JUTGamePad::PadStatus(m_padPort)->button & 15;
    }

    padStatus.button = mappedButtons;
    u32 stickButtons = m_mainStick.update(padStatus.stickX, padStatus.stickY, Clamped,
            WhichStick_MainStick, m_button);
    m_button.update(&padStatus, stickButtons << 0x18);
}
