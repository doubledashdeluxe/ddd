#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

typedef struct ARQRequest ARQRequest;

typedef void (*ARQCallback)(ARQRequest *task);

struct ARQRequest {
    struct ARQRequest *next;
    u32 owner;
    u32 type;
    u32 priority;
    u32 source;
    u32 dest;
    u32 length;
    ARQCallback callback;
};
size_assert(ARQRequest, 0x20);

REPLACE void ARQInit();
REPLACE void ARQPostRequest(ARQRequest *task, u32 owner, u32 type, u32 priority, u32 source,
        u32 dest, u32 length, ARQCallback callback);
