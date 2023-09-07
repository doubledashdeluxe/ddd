#include "MovieApp.hh"

#include "game/AppMgr.hh"

MovieApp *MovieApp::Create() {
    switch (AppMgr::CurrentApp()) {
    case AppMgr::KartAppEnum::Title:
        // Allow the opening movie to be played from the 'Title' scene
        return REPLACED(Create)();
    default:
        return nullptr;
    }
}
