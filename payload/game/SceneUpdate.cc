#include "SceneUpdate.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/KartLocale.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"

#include <cube/ChannelInstaller.hh>
#include <cube/Clock.hh>
#include <cube/DiscID.hh>
#include <cube/Platform.hh>
#include <cube/storage/Storage.hh>
extern "C" {
#include <dolphin/OSReset.h>
#include <dolphin/OSTime.h>
}
#include <formats/Version.hh>
extern "C" {
#include <monocypher/monocypher-ed25519.h>
}
#include <payload/Lock.hh>
#include <payload/ZIPFile.hh>
#include <payload/online/CubeClient.hh>
#include <portable/Align.hh>
#include <portable/Log.hh>

extern "C" {
#include <math.h>
#include <stdio.h>
}

SceneUpdate::SceneUpdate(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap)
    , m_heap(heap)
    , m_window(heap) {
    m_windowFrame = 0;
}

SceneUpdate::~SceneUpdate() {
    if (m_stack) {
        m_canceled = true;
        OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
        OSJoinThread(&m_thread, nullptr);
    }
}

void SceneUpdate::init() {
    m_readInfo.info.reset();
    m_readInfo.data.chunks.reset();
    m_writeInfo.serverIndex = OnlineInfo::Instance().m_serverIndex;
    m_writeInfo.info.region = DiscID::Get().gameID[3];
    m_writeInfo.info.platform = Platform::GetString();
    m_writeInfo.info.language = KartLocale::GetLanguage();
    m_writeInfo.data.reset();

    wait();
}

void SceneUpdate::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);

    m_window.draw();
}

void SceneUpdate::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();

    m_window.m_frame = 9.0f - fabs(9.0f - m_windowFrame);
    m_window.calc();

    {
        Lock<Mutex> lock(m_mutex);
        client->writeStateUpdate(m_writeInfo);
    }
}

bool SceneUpdate::clientStateServer(const ClientStateServerReadInfo & /* readInfo */) {
    return true;
}

bool SceneUpdate::clientStateUpdate(const ClientStateUpdateReadInfo &readInfo) {
    Lock<Mutex> lock(m_mutex);
    m_readInfo.version = readInfo.version;
    m_readInfo.info = readInfo.info;
    for (u32 i = 0; i < readInfo.data.chunks.count(); i++) {
        m_readInfo.data.chunks.pushBack(readInfo.data.chunks[i]);
    }
    return true;
}

void SceneUpdate::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneUpdate::wait() {
    m_state = &SceneUpdate::stateWait;
}

void SceneUpdate::info() {
    m_window.m_choiceType = 0;
    m_window.m_color = 0;
    m_window.m_choice = 0;
    char *text = m_window.getTextBox()->getString();
    snprintf(text, 256, "Would you like to install?\n%s -> %s%s", Version,
            m_readInfo.version.values(), m_readInfo.info->changelog.values());
    m_state = &SceneUpdate::stateInfo;
}

void SceneUpdate::data() {
    m_state = &SceneUpdate::stateData;
}

void SceneUpdate::completed() {
    m_state = &SceneUpdate::stateCompleted;
}

void SceneUpdate::nextScene() {
    m_state = &SceneUpdate::stateNextScene;
}

void SceneUpdate::reboot() {
    m_state = &SceneUpdate::stateReboot;
}

void SceneUpdate::stateWait() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_B) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        nextScene();
    } else if (m_readInfo.info) {
        info();
    }
}

void SceneUpdate::stateInfo() {
    if (!openWindow()) {
        return;
    }

    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        closeWindow();
        if (m_window.m_choice == 0) {
            data();
        } else {
            nextScene();
        }
    } else if (button.repeat() & (JUTGamePad::PAD_MSTICK_LEFT | JUTGamePad::PAD_MSTICK_RIGHT)) {
        m_window.m_choice ^= 1;
    }
}

void SceneUpdate::stateData() {
    if (openWindow()) {
        const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
        if (button.risingEdge() & PAD_BUTTON_A && !m_stack) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            closeWindow();
            nextScene();
        } else if (button.risingEdge() & PAD_BUTTON_B && m_stack) {
            m_canceled = true;
            OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
        }
    } else if (isWindowClosed()) {
        m_window.m_choiceType = 1;
        m_window.m_choice = 2;
        m_canceled = false;
        m_completed = false;
        m_version = m_readInfo.version;
        m_size = m_readInfo.info->size;
        m_alignedSize = AlignUp(m_size, UpdateChunkSize);
        m_chunkCount = m_alignedSize / UpdateChunkSize;
        m_downloadedChunkCount = 0;
        m_text[0] = '\0';
        OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
        u32 stackSize = 64 * 1024;
        m_stack.reset(new (m_heap, 0x8) u8[stackSize]);
        OSCreateThread(&m_thread, Update, this, m_stack.get() + stackSize, stackSize, 15, 0);
        OSResumeThread(&m_thread);
    }

    if (!m_stack) {
        return;
    }

    Lock<Mutex> lock(m_mutex);

    char *text = m_window.getTextBox()->getString();
    if (m_canceled) {
        snprintf(text, 256, "Canceling...");
    } else {
        snprintf(text, 256, "%s", m_text);
    }

    if (OSIsThreadTerminated(&m_thread)) {
        OSDetachThread(&m_thread);
        m_stack.reset();

        if (m_completed) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            closeWindow();
            completed();
            return;
        }

        if (m_canceled) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
            closeWindow();
            nextScene();
            return;
        }
    }

    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);

    Optional<ClientStateUpdateWriteInfo::Data> &data = m_writeInfo.data;
    if (!data) {
        return;
    }

    Ring<u16, MaxUpdateIndexCount> &indices = data->indices;
    indices.reset();
    if (m_downloadedChunkCount == m_chunkCount) {
        return;
    }

    // It's fine to ask multiple times for the same chunks if only a few are remaining!
    while (u16 *index = indices.emplaceFront()) {
        do {
            m_chunkIndex = m_chunkIndices[(m_chunkIndex + 1) % Count(m_chunkIndices)];
        } while (m_chunkIndex >= m_chunkCount);
        *index = m_chunkIndex;
    }
}

void SceneUpdate::stateCompleted() {
    if (openWindow()) {
        const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
        if (button.risingEdge() & PAD_BUTTON_A) {
            if (m_window.m_choice == 0) {
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
            } else {
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            }
            closeWindow();
            if (m_window.m_choice == 0) {
                GameAudio::Main::Instance()->fadeOutAll(15);
                System::GetDisplay()->startFadeOut(15);
                reboot();
            } else {
                nextScene();
            }
        } else if (button.repeat() & (JUTGamePad::PAD_MSTICK_LEFT | JUTGamePad::PAD_MSTICK_RIGHT)) {
            m_window.m_choice ^= 1;
        }
    } else if (isWindowClosed()) {
        m_window.m_choiceType = 0;
        m_window.m_choice = 0;
        char *text = m_window.getTextBox()->getString();
        snprintf(text, 256,
                "A restart is required to complete the\ninstall. Would you like to restart DDD "
                "now?");
    }
}

void SceneUpdate::stateNextScene() {
    if (!closeWindow()) {
        return;
    }

    u32 nextScene = SceneType::ServerSelect;
    if (!SequenceApp::Instance()->ready(nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(nextScene);
}

void SceneUpdate::stateReboot() {
    if (!closeWindow()) {
        return;
    }

    if (System::GetDisplay()->getFaderStatus() != JUTFader::Status::Out) {
        return;
    }

    s32 reset = Platform::IsGameCube() ? OS_RESET_HOTRESET : OS_RESET_RESTART;
    OSResetSystem(reset, 0, false);
}

bool SceneUpdate::isWindowClosed() const {
    return m_windowFrame == 0;
}

bool SceneUpdate::isWindowOpen() const {
    return m_windowFrame == 9;
}

bool SceneUpdate::openWindow() {
    if (isWindowOpen()) {
        return true;
    }

    m_windowFrame = (m_windowFrame + 1) % 18;
    return isWindowOpen();
}

bool SceneUpdate::closeWindow() {
    if (isWindowClosed()) {
        return true;
    }

    m_windowFrame = (m_windowFrame + 1) % 18;
    return isWindowClosed();
}

void SceneUpdate::update() {
    Storage::FileHandle file(ZIPPath, Storage::Mode::WriteAlways);

    if (!download(file)) {
        report("Download failed!");
        return;
    }

    if (!verify(file)) {
        report("Verification failed!");
        return;
    }

    if (!install(file)) {
        report("Installation failed!");
        return;
    }

    report("");
    m_completed = true;
}

bool SceneUpdate::download(Storage::FileHandle &file) {
    DEBUG("Downloading (%u)...", m_size);

    if (!file.truncate(m_alignedSize)) {
        return false;
    }

    m_writeInfo.data.emplace();
    for (u32 i = 0; i < Count(m_chunkIndices); i++) {
        m_chunkIndices[i] = i;
    }
    m_chunkIndex = Count(m_chunkIndices) - 1;

    for (s64 start = OSGetTime(); m_downloadedChunkCount < m_chunkCount;) {
        alignas(0x20) u8 chunks[48 * UpdateChunkSize];
        u32 count = 0, nextIndex;

        while (m_downloadedChunkCount < m_chunkCount && count * UpdateChunkSize < Count(chunks)) {
            if (m_canceled) {
                return false;
            }

            ClientStateUpdateReadInfo::Chunk *chunk;
            {
                Lock<Mutex> lock(m_mutex);
                chunk = m_readInfo.data.chunks.front();
            }

            if (!chunk) {
                if (count == 0) {
                    OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
                    continue;
                } else {
                    break;
                }
            }

            u16 index = chunk->index;
            if (m_chunkIndices[index % Count(m_chunkIndices)] == index) {
                if (count != 0 && index != nextIndex) {
                    break;
                }

                memcpy(chunks + count * UpdateChunkSize, chunk->chunk.values(),
                        chunk->chunk.count());

                count++;
                nextIndex = index + 1;

                m_chunkIndices[index % Count(m_chunkIndices)] += Count(m_chunkIndices);
                m_downloadedChunkCount++;

                u32 downloadedSize = m_downloadedChunkCount * UpdateChunkSize;
                u32 size = m_chunkCount * UpdateChunkSize;
                report("Downloading...", downloadedSize, size, start);
            }

            Lock<Mutex> lock(m_mutex);
            m_readInfo.data.chunks.popFront();
        }

        u32 size = count * UpdateChunkSize;
        u32 offset = (nextIndex - count) * UpdateChunkSize;
        if (!file.write(chunks, size, offset)) {
            return false;
        }
    }

    return file.truncate(m_size);
}

bool SceneUpdate::verify(Storage::FileHandle &file) {
    DEBUG("Verifying (%u)...", m_size);

    alignas(0x20) u8 signature[64];
    if (!file.read(signature, sizeof(signature), 0)) {
        return false;
    }

    crypto_sha512_ctx ctx;
    crypto_sha512_init(&ctx);
    s64 start = OSGetTime();
    for (u32 offset = 64; offset < m_size;) {
        if (m_canceled) {
            return false;
        }

        alignas(0x20) u8 chunk[4 * 1024];
        u32 chunkSize = Min(m_size - offset, sizeof(chunk));
        if (!file.read(chunk, chunkSize, offset)) {
            return false;
        }

        crypto_sha512_update(&ctx, chunk, chunkSize);
        offset += chunkSize;

        report("Verifying...", offset, m_size, start);
    }
    u8 hash[64];
    crypto_sha512_final(&ctx, hash);

    u8 pk[32] = {0x29, 0x4c, 0x41, 0xa8, 0x1e, 0x9c, 0xc3, 0xca, 0x58, 0x7c, 0xaa, 0xe2, 0xd0, 0xb0,
            0x64, 0x45, 0x7a, 0x4c, 0x48, 0x13, 0xfc, 0xcf, 0x6d, 0x45, 0xf3, 0xb1, 0x2c, 0x28,
            0x93, 0x97, 0xd2, 0x61};
    return !crypto_ed25519_ph_check(signature, pk, hash);
}

bool SceneUpdate::install(Storage::FileHandle &file) {
    report("Installing...");

    if (!RemoveAll(UpdatePath, Storage::Mode::RemoveAlways)) {
        return false;
    }
    if (!Storage::CreateDir(UpdatePath, Storage::Mode::WriteNew)) {
        return false;
    }

    ZIPFile zipFile(file);
    if (!zipFile.ok()) {
        return false;
    }

    bool hasVersion = false;

    ZIPFile::Iterator it(zipFile);
    for (; !it.done(); it.next()) {
        if (m_canceled) {
            return false;
        }

        ZIPFile::Reader reader(it);
        if (!reader.ok()) {
            return false;
        }

        const char *path = reader.cdNode()->path.values();
        const u8 *buffer;
        u32 size;
        if (!strcmp(path, "version.txt")) {
            // Without this check, a malicious server could trick users into downloading a different
            // version from what it advertises.

            if (!reader.read(buffer, size)) {
                return false;
            }

            u32 versionLength = strlen(m_version.values());
            if (size != versionLength + 1) {
                return false;
            }
            if (memcmp(buffer, m_version.values(), versionLength)) {
                return false;
            }

            hasVersion = true;
            continue;
        }

        if (!strcmp(path, "changelog.txt")) {
            continue;
        }

        {
            Lock<Mutex> lock(m_mutex);
            snprintf(m_text, Count(m_text), "Installing...\n%s", path);
        }

        DEBUG("Installing %s...", path);

        char filePath[256];
        snprintf(filePath, Count(filePath), "%s/%s", UpdatePath, path);
        for (char *c = filePath + strlen(UpdatePath); *c; c++) {
            if (*c != '/') {
                continue;
            }

            *c = '\0';
            if (!Storage::CreateDir(filePath, Storage::Mode::WriteAlways)) {
                return false;
            }
            *c = '/';
        }
        Storage::FileHandle file(filePath, Storage::Mode::WriteNew);
        for (u32 offset = 0; offset < *reader.size(); offset += size) {
            if (m_canceled) {
                return false;
            }

            if (!reader.read(buffer, size)) {
                return false;
            }
            if (!file.write(buffer, size, offset)) {
                return false;
            }
        }
    }
    if (!it.ok()) {
        return false;
    }

    if (!hasVersion) {
        return false;
    }

    if (m_canceled) {
        return false;
    }

    report("Installing...");

    if (!RemoveAll(BackupPath, Storage::Mode::RemoveAlways)) {
        return false;
    }
    if (!Rename(AppPath, BackupPath)) {
        return false;
    }
    if (!Rename(UpdatePath, AppPath)) {
        Storage::Rename(BackupPath, AppPath);
        return false;
    }
    if (!RemoveAll(BackupPath, Storage::Mode::RemoveExisting)) {
        return false;
    }

    if (Platform::IsGameCube()) {
        return true;
    }

    report("Installing...\nchannel.dol");

    Storage::FileHandle dolFile("main:/apps/ddd/boot.dol", Storage::Mode::Read);
    alignas(0x20) u8 dolHeader[0x100];
    if (!dolFile.read(dolHeader, sizeof(dolHeader), 0)) {
        return false;
    }

    u32 offset = Bytes::ReadBE<u32>(dolHeader, 0x1c);
    u32 size = Bytes::ReadBE<u32>(dolHeader, 0xac);
    return ChannelInstaller::Install(dolFile, size, offset);
}

void SceneUpdate::report(const char *text) {
    Lock<Mutex> lock(m_mutex);
    snprintf(m_text, Count(m_text), "%s", text);
}

void SceneUpdate::report(const char *text, u32 size, u32 totalSize, s64 start) {
    s64 duration = OSGetTime() - start;
    u32 throughput = Clock::SecondsToTicks(size) / duration;

    f32 scale = 1.0f / (1024.0f * 1024.0f);
    f32 scaledSize = size * scale;
    f32 scaledTotalSize = totalSize * scale;
    f32 scaledThroughput = throughput * scale;

    Lock<Mutex> lock(m_mutex);
    snprintf(m_text, Count(m_text), "%s\n%.2f MiB / %.2f MiB (%.2f MiB/s)", text, scaledSize,
            scaledTotalSize, scaledThroughput);
}

void *SceneUpdate::Update(void *param) {
    static_cast<SceneUpdate *>(param)->update();
    Remove(ZIPPath, Storage::Mode::RemoveAlways);
    RemoveAll(UpdatePath, Storage::Mode::RemoveAlways);
    return nullptr;
}

bool SceneUpdate::Rename(const char *srcPath, const char *dstPath) {
    DEBUG("Renaming %s to %s...", srcPath, dstPath);
    if (Storage::Rename(srcPath, dstPath)) {
        return true;
    }
    ERROR("Failed to rename %s to %s", srcPath, dstPath);
    return false;
}

bool SceneUpdate::Remove(const char *path, u32 mode) {
    DEBUG("Removing %s...", path);
    if (Storage::Remove(path, mode)) {
        return true;
    }
    ERROR("Failed to remove %s", path);
    return false;
}

bool SceneUpdate::RemoveAll(const char *path, u32 mode) {
    DEBUG("Removing %s...", path);
    if (Storage::RemoveAll(path, mode)) {
        return true;
    }
    ERROR("Failed to remove %s", path);
    return false;
}

const char SceneUpdate::ZIPPath[] = "main:/ddd/.update.zip";
const char SceneUpdate::UpdatePath[] = "main:/ddd/.update";
const char SceneUpdate::BackupPath[] = "main:/ddd/.backup";
const char SceneUpdate::AppPath[] = "main:/apps/ddd";
