#include "BattleName2D.hh"

BattleName2D *BattleName2D::Create(JKRArchive *archive) {
    if (!s_instance) {
        s_instance = new BattleName2D(archive);
    }
    return s_instance;
}

BattleName2D *BattleName2D::Instance() {
    return s_instance;
}
