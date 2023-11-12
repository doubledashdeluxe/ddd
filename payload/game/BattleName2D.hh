#pragma once

#include <jsystem/JKRArchive.hh>
#include <jsystem/ResTIMG.hh>

class BattleName2D {
public:
    ~BattleName2D();
    ResTIMG *getBattleNameTexture(u32 battleMode);

    static BattleName2D *Create(JKRArchive *archive);
    static BattleName2D *Instance();

private:
    BattleName2D(JKRArchive *archive);

    u8 _0[0xc - 0x0];

    static BattleName2D *s_instance;
};
size_assert(BattleName2D, 0xc);
