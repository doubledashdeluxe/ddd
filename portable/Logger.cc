#include "Logger.hh"

extern "C" {
#include <assert.h>
}

Logger *Logger::Instance() {
    return s_instance;
}

Logger::Logger() {
    assert(!s_instance);
    s_instance = this;
}

Logger::~Logger() {}

Logger *Logger::s_instance = nullptr;
