#include "CanNotSaveG2D.hh"

#include "game/J2DManager.hh"
#include "game/RaceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>

void CanNotSaveG2D::setupOnline() {
    if (!RaceInfo::Instance().isBattle()) {
        JKRArchive *archive = J2DManager::Instance()->archive();
        m_anmTransform = J2DAnmLoaderDataBase::Load("Live.bck", archive);
        m_screen->setAnimation(m_anmTransform);
    }

    J2DPicture *picture = m_screen->search("NoSaveM")->downcast<J2DPicture>();
    picture->changeTexture("Mozi_Live.bti", 0);

    m_isActive = true;
}
