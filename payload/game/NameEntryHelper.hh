#pragma once

#include <jsystem/J2DScreen.hh>
#include <jsystem/JKRHeap.hh>
#include <jsystem/JUTGamePad.hh>

class NameEntryHelper {
public:
    NameEntryHelper(J2DPane **panes, char *name, JKRHeap *heap, JUTGamePad *pad, bool canCancel);

    bool wasSelected() const;
    bool wasCanceled() const;
    void init(const char *name);
    void startEdit();
    void stopEdit();
    void draw(J2DGraphContext *graphContext);
    void calc();

private:
    class NameEntryDrum {
    public:
        void selectOut();

    private:
        u8 _00[0x68 - 0x00];
    };
    size_assert(NameEntryDrum, 0x68);

    class State {
    public:
        enum {
            Inactive = 0,
            Activated = 1,
            Active = 2,
            Selected = 3,
            Canceled = 4,
        };

    private:
        State();
    };

    NameEntryDrum *m_drums[3];
    u8 _0c[0x10 - 0x0c];
    s16 m_index;
    u8 _12[0x14 - 0x12];
    s32 m_state;
    u8 _18[0x1c - 0x18];
};
size_assert(NameEntryHelper, 0x1c);
