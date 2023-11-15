#pragma once

#include <jsystem/JKRHeap.hh>

class CharacterSelect3D {
public:
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
