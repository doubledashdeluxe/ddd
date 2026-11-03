#pragma once

extern "C" {
#include <dolphin/PAD.h>
}

class JUTGamePad {
public:
    enum {
        PAD_MSTICK_LEFT = 1 << 24,
        PAD_MSTICK_RIGHT = 1 << 25,
        PAD_MSTICK_DOWN = 1 << 26,
        PAD_MSTICK_UP = 1 << 27,
    };

    // Copied from the decomp
    enum EStickMode {
        NonClamped = 0,
        Clamped = 1,
    };

    enum EWhichStick {
        WhichStick_MainStick = 0,
        WhichStick_SubStick = 1,
    };

    class CButton {
    public:
        CButton();

        u32 level() const;
        u32 risingEdge() const;
        u32 fallingEdge() const;
        u32 repeat() const;
        void update(const PADStatus *padStatus, u32 stickButtons);

    private:
        u32 m_level;
        u32 m_risingEdge;
        u32 m_fallingEdge;
        u8 _0c[0x18 - 0x0c];
        u32 m_repeat;
        u8 _1c[0x30 - 0x1c];
    };
    size_assert(CButton, 0x30);

    class CStick {
    public:
        f32 x() const;
        f32 y() const;
        u32 update(s8 stickX, s8 stickY, EStickMode stickMode, EWhichStick whichStick,
                CButton button);

    private:
        f32 m_x;
        f32 m_y;
        u8 _08[0x10 - 0x08];
    };
    size_assert(CStick, 0x10);

    JUTGamePad(s32 padPort);
    virtual ~JUTGamePad();

    const CButton &button() const;
    const CStick &mainStick() const;

    void startMotor();
    void stopMotor(bool hard);

    static PADStatus *PadStatus(u32 index);

private:
    class CRumble {
    public:
        static void StartMotor(s32 chan);
        static void StopMotor(s32 chan, bool hard);

    private:
        u8 _00[0x14 - 0x00];
    };
    size_assert(CRumble, 0x14);

    u8 _04[0x18 - 0x04];

protected:
    CButton m_button;
    CStick m_mainStick;

private:
    u8 _58[0x68 - 0x58];
    CRumble m_rumble;
    s16 m_chan;
    u8 _7e[0xa8 - 0x7e];

    static PADStatus s_padStatus[4];
};
size_assert(JUTGamePad, 0xa8);
