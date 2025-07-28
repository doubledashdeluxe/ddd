#include "AwardApp.hh"

AwardApp::AwardApp() : GameApp(0x600000, "Award", nullptr), m_state(0) {}

AwardApp *AwardApp::Instance() {
    return s_instance;
}
