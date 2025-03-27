#include <common/Dolphin.hh>
#define SNITCH_IMPLEMENTATION
#include <snitch/snitch_all.hpp>

#include <array>
#include <cstdio>
#include <cstring>

static bool exists;
static std::array<char, 64> versionString;

namespace IOS {

Resource::Resource(const char *path, u32 mode, bool /* isBlocking */) : m_fd(-1) {
    open(path, mode);
}

Resource::~Resource() = default;

s32 Resource::ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs) {
    if (ioctlv != 0x2 || inputCount != 0 || outputCount != 1 || !pairs) {
        return -4;
    }

    memcpy(pairs[0].data, versionString.data(), versionString.size());
    return 0;
}

s32 Resource::open(const char *path, u32 /* mode */) {
    m_fd = !strncmp(path, "/dev/dolphin", strlen("/dev/dolphin")) && exists ? 0 : -1;
    return m_fd >= 0;
}

} // namespace IOS

TEST_CASE("Dolphin::ok") {
    SECTION("Doesn't exist") {
        exists = false;

        Dolphin dolphin;

        CHECK_FALSE(dolphin.ok());
    }

    SECTION("Exists") {
        exists = true;

        Dolphin dolphin;

        CHECK(dolphin.ok());
    }
}

TEST_CASE("Dolphin::getVersion") {
    exists = true;

    SECTION("Invalid") {
        memset(versionString.data(), '1', versionString.size());

        Dolphin dolphin;
        CHECK(dolphin.ok());
        Dolphin::Version actualVersion;
        bool result = dolphin.getVersion(actualVersion);

        CHECK_FALSE(result);
    }

    SECTION("Valid") {
        Dolphin::Version expectedVersion{5, 0, 20288};
        memset(versionString.data(), '\0', versionString.size());
        snprintf(versionString.data(), versionString.size(), "%u.%u-%u", expectedVersion.major,
                expectedVersion.minor, expectedVersion.patch);

        Dolphin dolphin;
        CHECK(dolphin.ok());
        Dolphin::Version actualVersion;
        bool result = dolphin.getVersion(actualVersion);

        CHECK(result);
        CHECK(actualVersion == expectedVersion);
    }
}
