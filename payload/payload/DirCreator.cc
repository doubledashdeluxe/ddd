#include "DirCreator.hh"

#include <cube/Arena.hh>

extern "C" {
#include <string.h>
}

void DirCreator::Init() {
    s_instance = new (MEM1Arena::Instance(), -0x4) DirCreator;
}

DirCreator::DirCreator() {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 22,
            0);
    OSResumeThread(&m_thread);
}

void DirCreator::onAdd(const char *prefix) {
    if (strcmp(prefix, "main:")) {
        return;
    }

    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void DirCreator::onRemove(const char * /* prefix */) {}

void *DirCreator::run() {
    while (true) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
        Storage::CreateDir("main:/ddd", Storage::Mode::WriteAlways);
        Storage::CreateDir("main:/ddd/assets", Storage::Mode::WriteAlways);
    }
}

void *DirCreator::Run(void *param) {
    return reinterpret_cast<DirCreator *>(param)->run();
}

DirCreator *DirCreator::s_instance = nullptr;
