#include <portable/DolphinVersion.hh>
#include <snitch/snitch_all.hpp>

#include <cstdio>
#include <cstring>

TEST_CASE("DolphinVersion") {
    Array<char, 64> versionString;
    DolphinVersion oldVersion{5, 0, 20288};
    DolphinVersion newVersion{2412, 0, 0};

    SECTION("Read") {
        SECTION("Invalid") {
            versionString.fill('1');

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK_FALSE(result);
        }

        SECTION("Valid old") {
            versionString.fill('\0');
            snprintf(versionString.values(), versionString.count(), "%u.%u-%u", oldVersion.major,
                    oldVersion.minor, oldVersion.patch);

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK(result);
            CHECK(version == oldVersion);
        }

        SECTION("Valid new") {
            versionString.fill('\0');
            snprintf(versionString.values(), versionString.count(), "%u", newVersion.major);

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK(result);
            CHECK(version == newVersion);
        }
    }

    SECTION("operator==") {
        CHECK(oldVersion == oldVersion);
    }

    SECTION("operator!=") {
        CHECK(oldVersion != newVersion);
    }

    SECTION("operator<") {
        CHECK(oldVersion < newVersion);
    }

    SECTION("operator>") {
        CHECK(newVersion > oldVersion);
    }

    SECTION("operator<=") {
        CHECK(oldVersion <= oldVersion);
    }

    SECTION("operator>=") {
        CHECK(oldVersion >= oldVersion);
    }
}
