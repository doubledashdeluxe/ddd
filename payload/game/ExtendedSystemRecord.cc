#include "ExtendedSystemRecord.hh"

ExtendedSystemRecord &ExtendedSystemRecord::Instance() {
    return s_instance;
}

ExtendedSystemRecord ExtendedSystemRecord::s_instance;
