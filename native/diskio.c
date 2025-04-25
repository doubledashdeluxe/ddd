// clang-format off
#include <ff/ff.h>
#include <ff/diskio.h>
// clang-format on

DSTATUS disk_initialize(BYTE /* pdrv */) {
    return 0;
}

DSTATUS disk_status(BYTE /* pdrv */) {
    return 0;
}

DRESULT disk_read(BYTE /* pdrv */, BYTE * /* buff */, LBA_t /* sector */, UINT /* count */) {
    return RES_ERROR;
}

DRESULT disk_write(BYTE /* pdrv */, const BYTE * /* buff */, LBA_t /* sector */, UINT /* count */) {
    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE /* pdrv */, BYTE /* cmd */, void * /* buff */) {
    return RES_ERROR;
}

DWORD get_fattime(void) {
    return 0;
}
