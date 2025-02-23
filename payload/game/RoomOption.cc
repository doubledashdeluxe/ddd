#include "RoomOption.hh"

u32 RoomOption::ValueCount(u32 roomOption) {
    static const u32 valueCounts[] = {2, 3, 4, 4, 10, 13, 13, 3, 3};
    return valueCounts[roomOption];
}
