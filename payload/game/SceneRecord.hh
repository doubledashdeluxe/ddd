#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class SceneRecord : public Scene {
public:
    SceneRecord(JKRArchive *archive, JKRHeap *heap);
    ~SceneRecord() override;
    void REPLACED(init)();
    REPLACE void init() override;
    void draw() override;
    void calc() override;

private:
    u8 _00c[0x080 - 0x00c];
    J2DScreen *m_backgroundScreen;
    u8 _084[0x704 - 0x084];
};
size_assert(SceneRecord, 0x704);
