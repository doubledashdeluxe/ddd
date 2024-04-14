#include "MenuBackground.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

void MenuBackground::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

void MenuBackground::calc() {
    m_screen.search("Back")->setHasARScale(false, false);
    m_screen.search("B_mozi")->setHasARScale(false, false);

    REPLACED(calc)();
}

MenuBackground *MenuBackground::Create(JKRArchive *archive) {
    if (!s_instance) {
        s_instance = new MenuBackground(archive);
    }
    return s_instance;
}

void MenuBackground::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

MenuBackground *MenuBackground::Instance() {
    return s_instance;
}

MenuBackground::~MenuBackground() {}
