#include "AssetsDirCreator.hh"

#include <common/Arena.hh>

extern "C" {
#include <string.h>
}

void AssetsDirCreator::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x4) AssetsDirCreator;
}

AssetsDirCreator::AssetsDirCreator() {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 22,
            0);
    OSResumeThread(&m_thread);
}

void AssetsDirCreator::onAdd(const char *prefix) {
    if (strcmp(prefix, "main:")) {
        return;
    }

    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void AssetsDirCreator::onRemove(const char * /* prefix */) {}

void *AssetsDirCreator::run() {
    while (true) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
        Storage::CreateDir("main:/ddd/assets", Storage::Mode::WriteAlways);
    }
}

void *AssetsDirCreator::Run(void *param) {
    return reinterpret_cast<AssetsDirCreator *>(param)->run();
}

AssetsDirCreator *AssetsDirCreator::s_instance = nullptr;
