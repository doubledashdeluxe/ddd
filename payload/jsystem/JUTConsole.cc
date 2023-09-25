#include "JUTConsole.hh"

extern "C" {
#include <dolphin/OSError.h>
}

void JUTConsole::print(const char *message) {
    if (m_outputFlag & JUTConsole::OutputFlag::OSReport) {
        OSReport("%s", message);
    }

    REPLACED(print)(message);
}
