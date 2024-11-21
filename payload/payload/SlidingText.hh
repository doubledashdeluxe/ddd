#pragma once

#include <jsystem/J2DScreen.hh>

class SlidingText {
public:
    SlidingText();
    ~SlidingText();
    void refresh(u32 offset, u32 partCount, u32 pictureCount, J2DScreen &screen,
            const char *prefix);

private:
    virtual const char *getPart(u32 i) = 0;
    virtual void setAnmTransformFrame(u8 anmTransformFrame) = 0;
    virtual void setAlpha(u8 alpha) = 0;
};
