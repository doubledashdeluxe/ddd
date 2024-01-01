#include <common/Dolphin.hh>
#include <lest.hpp>

#include <cstring>

static bool exists;
static Array<char, 64> expectedVersion;

namespace IOS {

Resource::Resource(const char *path, u32 mode) : m_fd(-1) {
    open(path, mode);
}

Resource::~Resource() = default;

s32 Resource::ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs) {
    if (ioctlv != 0x2 || inputCount != 0 || outputCount != 1 || !pairs) {
        return -4;
    }

    memcpy(pairs[0].data, expectedVersion.values(), expectedVersion.count());
    return 0;
}

s32 Resource::open(const char *path, u32 /* mode */) {
    m_fd = !strncmp(path, "/dev/dolphin", strlen("/dev/dolphin")) && exists ? 0 : -1;
    return m_fd >= 0;
}

} // namespace IOS

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("ok") {
    SETUP("Placeholder") {
        SECTION("Doesn't exist") {
            exists = false;

            Dolphin dolphin;

            EXPECT(!dolphin.ok());
        }

        SECTION("Exists") {
            exists = true;

            Dolphin dolphin;

            EXPECT(dolphin.ok());
        }
    }
}

CASE("getVersion") {
    SETUP("Exists") {
        exists = true;

        SECTION("Invalid") {
            memset(expectedVersion.values(), '1', expectedVersion.count());

            Dolphin dolphin;
            EXPECT(dolphin.ok());
            Array<char, 64> actualVersion;
            bool result = dolphin.getVersion(actualVersion);

            EXPECT(!result);
        }

        SECTION("Valid") {
            strncpy(expectedVersion.values(), "5.0-20288", expectedVersion.count());

            Dolphin dolphin;
            EXPECT(dolphin.ok());
            Array<char, 64> actualVersion;
            bool result = dolphin.getVersion(actualVersion);

            EXPECT(result);
            EXPECT(!memcmp(actualVersion.values(), expectedVersion.values(),
                    actualVersion.count()));
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
