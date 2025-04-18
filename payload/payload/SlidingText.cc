#include "SlidingText.hh"

#include <game/Kart2DCommon.hh>
#include <jsystem/J2DPicture.hh>
#include <portable/UTF8.hh>

SlidingText::SlidingText() {}

SlidingText::~SlidingText() {}

void SlidingText::refresh(u32 offset, u32 partCount, u32 pictureCount, J2DScreen &screen,
        const char *prefix) {
    const char *sep = " / ";
    u32 sepLength = UTF8::Length(sep);
    u32 length = 0;
    for (u32 i = 0; i < partCount; i++) {
        length += UTF8::Length(getPart(i)) + sepLength;
    }
    if (length + 1 <= pictureCount + sepLength) {
        length -= sepLength;
        setAnmTransformFrame(0);
        setAlpha(0);
    } else {
        setAnmTransformFrame(offset / 5 % 12);
        setAlpha(offset / 4 % 15 * 17);
        offset = offset / 60 % length;
    }
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0, j = 0; i < partCount; i++) {
        for (u32 k = 0; k < 2; k++) {
            for (const char *p = k == 0 ? getPart(i) : sep; *p; j++) {
                u32 l = j;
                if (length + 1 > pictureCount) {
                    l = (length + j - offset) % length;
                }
                u32 c = UTF8::Next(p);
                if (l < pictureCount) {
                    J2DPicture *picture = screen.search("%s%u", prefix, l)->downcast<J2DPicture>();
                    picture->changeTexture(kart2DCommon->getUnicodeTexture(c), 0);
                }
            }
        }
    }
    for (u32 i = 0; i < pictureCount; i++) {
        J2DPicture *picture = screen.search("%s%u", prefix, i)->downcast<J2DPicture>();
        picture->m_isVisible = i < length;
    }
}
