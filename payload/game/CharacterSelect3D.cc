#include "CharacterSelect3D.hh"

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
