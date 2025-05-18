#include "PadRecorder.hh"

PadRecorder *PadRecorder::Instance() {
    return s_instance;
}
