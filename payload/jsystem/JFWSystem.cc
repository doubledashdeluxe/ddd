#include "JFWSystem.hh"

void JFWSystem::Init() {
    s_systemHeapSize = 832 * 1024;

    REPLACED(Init)();
}
