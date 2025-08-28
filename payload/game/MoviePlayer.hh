#pragma once

#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class MoviePlayer {
public:
    static void REPLACED(Create)(JKRHeap *heap);
    REPLACE static void Create(JKRHeap *heap);
    static MoviePlayer *Instance();
    static void DrawDone();

private:
    MoviePlayer();
    virtual ~MoviePlayer();

    u8 _04[0x48 - 0x04];

    static MoviePlayer *s_instance;
};
size_assert(MoviePlayer, 0x48);
