#include <portable/network/DNS.hh>

#include <cassert>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    u32 offset = 0, nameLength;
    DNS::Name name;
    if (!DNS::ReadName(data, size, offset, name, nameLength)) {
        return 0;
    }

    std::vector<u8> otherData(nameLength + 2);
    offset = 0;
    if (!DNS::WriteName(otherData.data(), otherData.size(), offset, name, nameLength)) {
        return 0;
    }

    offset = 0;
    DNS::Name otherName;
    assert(DNS::ReadName(otherData.data(), otherData.size(), offset, otherName, nameLength));
    assert(!strcmp(name.values(), otherName.values()));
    return 0;
}
