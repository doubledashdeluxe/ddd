#include "JUTProcBar.hh"

#include <payload/PerfOverlay.hh>

void JUTProcBar::draw() {
    PerfOverlay::Instance()->draw();
}
