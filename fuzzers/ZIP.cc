#include <native/FakeZIP.hh>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    std::vector<u8> zipData(data, data + size);
    FakeZIP zip(zipData);
    if (!zip.ok()) {
        return 0;
    }
    ZIP::Reader reader(zip, "/a");
    if (!reader.ok()) {
        return 0;
    }
    std::vector<u8> fileData(*reader.size());
    for (u32 offset = 0; offset < *reader.size();) {
        const u8 *chunk;
        u32 chunkSize;
        if (!reader.read(chunk, chunkSize)) {
            return 0;
        }
        fileData.insert(fileData.end(), chunk, chunk + chunkSize);
        offset += chunkSize;
    }
    return 0;
}
