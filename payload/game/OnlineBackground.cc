#include "OnlineBackground.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

void OnlineBackground::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

void OnlineBackground::calc() {
    m_anmTransformFrame = (m_anmTransformFrame + 1) % 97;
    m_anmTextureSRTKeyFrame = (m_anmTextureSRTKeyFrame + 1) % 360;

    m_anmTransform->m_frame = m_anmTransformFrame;
    m_anmTextureSRTKey->m_frame = m_anmTextureSRTKeyFrame;
    m_anmColor->m_frame = m_anmColorFrame;

    m_screen.animation();
}

OnlineBackground *OnlineBackground::Create(JKRArchive *archive) {
    if (!s_instance) {
        s_instance = new OnlineBackground(archive);
    }
    return s_instance;
}

void OnlineBackground::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

OnlineBackground *OnlineBackground::Instance() {
    return s_instance;
}

OnlineBackground::OnlineBackground(JKRArchive *archive) {
    m_screen.setPriority("Back1.blo", 0x20000, archive);

    m_screen.setHasARTrans(false, true);
    m_screen.setHasARShift(false, true);
    m_screen.setHasARScale(false, true);
    m_screen.setHasARTexCoords(false, true);

    m_anmTransform = J2DAnmLoaderDataBase::Load("Back1.bck", archive);
    m_screen.setAnimation(m_anmTransform);
    m_anmTextureSRTKey = J2DAnmLoaderDataBase::Load("Back1.btk", archive);
    m_anmTextureSRTKey->searchUpdateMaterialID(&m_screen);
    m_screen.setAnimation(m_anmTextureSRTKey);
    m_anmColor = J2DAnmLoaderDataBase::Load("Back1.bpk", archive);
    m_anmColor->searchUpdateMaterialID(&m_screen);
    m_screen.setAnimation(m_anmColor);

    m_anmTransformFrame = 0;
    m_anmTextureSRTKeyFrame = 0;
    m_anmColorFrame = 0;
}

OnlineBackground::~OnlineBackground() {}

OnlineBackground *OnlineBackground::s_instance = nullptr;
