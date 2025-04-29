#include <native/FakeSZSReader.hh>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    std::vector<u8> compressed(data, data + size);
    FakeSZSReader reader(&compressed);
    reader.read();
    return 0;
}
