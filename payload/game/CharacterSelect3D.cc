#include "CharacterSelect3D.hh"

extern "C" {
#include <dolphin/GXTransform.h>
}

void CharacterSelect3D::draw(s32 statusIndex, f32 aspect) {
    u32 scissorX, scissorY, scissorWidth, scissorHeight;
    GXGetScissor(&scissorX, &scissorY, &scissorWidth, &scissorHeight);
    f32 x = scissorX, w = scissorWidth;
    x += 0.5f * w;
    x -= 0.5f * 608.0f;
    x *= J2DPane::GetARScale();
    x += 0.5f * 608.0f;
    w *= J2DPane::GetARScale();
    x -= 0.5f * w;
    GXSetViewport(x, scissorY, w, scissorHeight, 0.0f, 1.0f);

    REPLACED(draw)(statusIndex, aspect);
}

CharacterSelect3D *CharacterSelect3D::Create(JKRHeap *heap) {
    if (!s_instance) {
        s_instance = new (heap, 0x4) CharacterSelect3D(heap);
    }
    return s_instance;
}

void CharacterSelect3D::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

CharacterSelect3D *CharacterSelect3D::Instance() {
    return s_instance;
}
