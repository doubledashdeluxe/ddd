#include "LogFile.hh"

#include "payload/Lock.hh"

#include <common/Clock.hh>
#include <common/Log.hh>
extern "C" {
#include <dolphin/OSTime.h>

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
}

void LogFile::Init() {
    OSCreateThread(&s_thread, Run, nullptr, s_stack.values() + s_stack.count(), s_stack.count(), 31,
            0);
    OSResumeThread(&s_thread);
}

void LogFile::VPrintf(const char *format, va_list vlist) {
    Lock<NoInterrupts> lock;

    u16 maxLength = s_buffers[0].count() - s_offset;
    s64 currentTime = OSGetSystemTime();
    if (!s_startTimeIsValid) {
        s_startTime = currentTime;
        s_startTimeIsValid = true;
    }
    u64 seconds = Clock::TicksToSeconds(currentTime - s_startTime);
    u32 milliseconds = Clock::TicksToMilliseconds(currentTime - s_startTime) % 1000;
    u32 length = snprintf(s_buffers[s_index].values() + s_offset, maxLength, "[%" PRIu64 ".%03u] ",
            seconds, milliseconds);
    if (length >= maxLength) {
        s_offset = s_buffers[0].count();
        return;
    }
    s_offset += length;
    maxLength -= length;
    length = vsnprintf(s_buffers[s_index].values() + s_offset, maxLength, format, vlist);
    if (length >= maxLength) {
        s_offset = s_buffers[0].count();
        return;
    }
    s_offset += length;
}

void *LogFile::Run(void * /* param */) {
    bool isReady = true;
    u8 index;
    u16 offset;

    for (;; Clock::WaitMilliseconds(1000)) {
        RemoveOldLogFiles();

        Storage::CreateDir("main:/ddd/logs", Storage::Mode::WriteAlways);

        OSCalendarTime time;
        OSTicksToCalendarTime(OSGetTime(), &time);

        Array<char, 256> path;
        s32 length = snprintf(path.values(), path.count(),
                "main:/ddd/logs/%04d-%02d-%02d-%02d-%02d-%02d.log", time.year, time.mon + 1,
                time.mday, time.hour, time.min, time.sec);
        if (length < 0 || static_cast<size_t>(length) >= path.count()) {
            continue;
        }

        Storage::FileHandle file(path.values(), Storage::Mode::WriteAlways);

        for (;; Clock::WaitMilliseconds(1000)) {
            if (isReady) {
                Lock<NoInterrupts> lock;

                if (s_offset == 0) {
                    continue;
                }

                index = s_index;
                s_index ^= 1;

                offset = s_offset;
                s_offset = 0;
            }
            isReady = false;

            u64 fileSize;
            if (!file.size(fileSize)) {
                break;
            }
            if (!file.write(s_buffers[index].values(), offset, fileSize)) {
                break;
            }
            if (!file.sync()) {
                break;
            }

            isReady = true;
        }
    }
}

void LogFile::RemoveOldLogFiles() {
    Storage::DirHandle dir("main:/ddd/logs");
    for (Storage::NodeInfo nodeInfo; dir.read(nodeInfo);) {
        if (!IsValidLogFile(nodeInfo)) {
            continue;
        }
        if (!ShouldRemoveLogFile(nodeInfo)) {
            continue;
        }

        Array<char, 256> path;
        s32 length =
                snprintf(path.values(), path.count(), "main:/ddd/logs/%s", nodeInfo.name.values());
        if (length < 0 || static_cast<size_t>(length) >= path.count()) {
            continue;
        }

        if (!Storage::Remove(path.values(), Storage::Mode::RemoveExisting)) {
            DEBUG("Failed to remove the log file '%s'!", path.values());
        }
    }
}

bool LogFile::IsValidLogFile(const Storage::NodeInfo &nodeInfo) {
    if (nodeInfo.type != Storage::NodeType::File) {
        return false;
    }

    const char *pattern = "DDDD-DD-DD-DD-DD-DD.log";
    if (strlen(nodeInfo.name.values()) != strlen(pattern)) {
        return false;
    }

    for (u32 i = 0; i < strlen(pattern); i++) {
        if (pattern[i] == 'D') {
            if (!isdigit(nodeInfo.name[i])) {
                return false;
            }
        } else {
            if (nodeInfo.name[i] != pattern[i]) {
                return false;
            }
        }
    }

    return true;
}

bool LogFile::ShouldRemoveLogFile(const Storage::NodeInfo &nodeInfo) {
    s64 oneDay = Clock::SecondsToTicks(24 * 60 * 60);
    s64 retention = 7 * oneDay;
    OSCalendarTime limit;
    OSTicksToCalendarTime(OSGetTime() - retention, &limit);

    s32 year = 0;
    for (u32 i = 0; i < 4; i++) {
        year = year * 10 + (nodeInfo.name[i] - '0');
    }
    if (year < limit.year) {
        return true;
    }

    s32 month = 0;
    for (u32 i = 5; i < 7; i++) {
        month = month * 10 + (nodeInfo.name[i] - '0');
    }
    if (month < limit.mon + 1) {
        return true;
    }

    s32 day = 0;
    for (u32 i = 8; i < 10; i++) {
        day = day * 10 + (nodeInfo.name[i] - '0');
    }
    if (day < limit.mday) {
        return true;
    }

    return false;
}

Array<u8, 8 * 1024> LogFile::s_stack;
OSThread LogFile::s_thread;
Array<Array<char, 0x4000>, 2> LogFile::s_buffers;
bool LogFile::s_startTimeIsValid = false;
s64 LogFile::s_startTime;
u8 LogFile::s_index = 0;
u16 LogFile::s_offset = 0;
