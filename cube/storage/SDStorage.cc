// clang-format off
//
// Resources:
// - https://www.sdcard.org/downloads/pls
//
// clang-format on

#include "SDStorage.hh"

SDStorage::SDStorage(Mutex *mutex) : FATStorage(mutex) {}
