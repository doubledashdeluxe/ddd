#pragma once

#include <jsystem/J2DPane.hh>
#include <jsystem/JKRHeap.hh>
#include <jsystem/TVec3.hh>
#include <payload/Replace.hh>

class CharacterSelect3D {
public:
    void init();
    void REPLACED(draw)(s32 statusIndex, f32 aspect);
    REPLACE void draw(s32 statusIndex, f32 aspect);
    void calc();
    void setCharacter(s32 statusIndex, s32 sideIndex, s32 characterID, Vec3f translation,
            Vec3f rotation, f32 scale);
    void setCharacterStatus(s32 statusIndex, s32 sideIndex, s32 status);
    void setKart(s32 statusIndex, s32 kartID, Vec3f translation, Vec3f rotation, f32 scale);
    bool isCancel(s32 statusIndex);
    bool isNext(s32 statusIndex);

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
