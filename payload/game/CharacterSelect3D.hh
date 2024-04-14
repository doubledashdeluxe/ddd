#pragma once

#include <jsystem/J2DPane.hh>
#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class CharacterSelect3D {
public:
    void REPLACED(draw)(s32 index, f32 aspect);
    REPLACE void draw(s32 index, f32 aspect);

    static CharacterSelect3D *Create(JKRHeap *heap);
    static void Destroy();
    static CharacterSelect3D *Instance();

private:
    CharacterSelect3D(JKRHeap *heap);
    ~CharacterSelect3D();

    u8 _0000[0x1018 - 0x0000];

    static CharacterSelect3D *s_instance;
};
size_assert(CharacterSelect3D, 0x1018);
