#include <portable/CString.hh>
#include <snitch/snitch_all.hpp>

#include <string>

TEST_CASE("CString") {
    using std::string_literals::operator""s;

    SECTION("Memcmp") {
        char s[] = "xyxz";
        CHECK(CString::Memcmp(s + 0, s + 0, 0) == 0);
        CHECK(CString::Memcmp(s + 0, s + 0, 1) == 0);
        CHECK(CString::Memcmp(s + 0, s + 2, 0) == 0);
        CHECK(CString::Memcmp(s + 2, s + 0, 0) == 0);
        CHECK(CString::Memcmp(s + 0, s + 2, 1) == 0);
        CHECK(CString::Memcmp(s + 2, s + 0, 1) == 0);
        CHECK(CString::Memcmp(s + 0, s + 2, 2) < 0);
        CHECK(CString::Memcmp(s + 2, s + 0, 2) > 0);
    }

    SECTION("Memcpy") {
        char s[] = "abcdef";
        CHECK(CString::Memcpy(s + 0, s + 0, 0) == s + 0);
        CHECK(s == "abcdef"s);
        CHECK(CString::Memcpy(s + 2, s + 0, 2) == s + 2);
        CHECK(s == "ababef"s);
        CHECK(CString::Memcpy(s + 2, s + 4, 2) == s + 2);
        CHECK(s == "abefef"s);
    }

    SECTION("Memmove") {
        char s[] = "abcdef";
        CHECK(CString::Memmove(s + 0, s + 0, 0) == s + 0);
        CHECK(s == "abcdef"s);
        CHECK(CString::Memmove(s + 2, s + 0, 4) == s + 2);
        CHECK(s == "ababcd"s);
        CHECK(CString::Memmove(s + 0, s + 2, 4) == s + 0);
        CHECK(s == "abcdcd"s);
    }

    SECTION("Memset") {
        char s[] = "abc";
        CHECK(CString::Memset(s + 0, 'd', 0) == s + 0);
        CHECK(s == "abc"s);
        CHECK(CString::Memset(s + 0, 'd', 3) == s + 0);
        CHECK(s == "ddd"s);
    }

    SECTION("Strchr") {
        char s[] = "xyxz";
        CHECK(CString::Strchr(s + 0, '\0') == s + 4);
        CHECK(CString::Strchr(s + 0, 'w') == nullptr);
        CHECK(CString::Strchr(s + 0, 'x') == s + 0);
        CHECK(CString::Strchr(s + 0, 'y') == s + 1);
        CHECK(CString::Strchr(s + 0, 'z') == s + 3);
    }

    SECTION("Strcmp") {
        char s[] = "xyxz";
        CHECK(CString::Strcmp(s + 0, s + 0) == 0);
        CHECK(CString::Strcmp(s + 0, s + 2) < 0);
        CHECK(CString::Strcmp(s + 2, s + 0) > 0);
    }

    SECTION("Strlen") {
        CHECK(CString::Strlen("") == 0);
        CHECK(CString::Strlen("a") == 1);
        CHECK(CString::Strlen("ab") == 2);
    }

    SECTION("Strncmp") {
        char s[] = "xyxz";
        CHECK(CString::Strncmp(s + 0, s + 0, 0) == 0);
        CHECK(CString::Strncmp(s + 0, s + 0, 1) == 0);
        CHECK(CString::Strncmp(s + 0, s + 0, 4) == 0);
        CHECK(CString::Strncmp(s + 0, s + 0, 5) == 0);
        CHECK(CString::Strncmp(s + 0, s + 0, 6) == 0);
        CHECK(CString::Strncmp(s + 0, s + 2, 0) == 0);
        CHECK(CString::Strncmp(s + 2, s + 0, 0) == 0);
        CHECK(CString::Strncmp(s + 0, s + 2, 1) == 0);
        CHECK(CString::Strncmp(s + 2, s + 0, 1) == 0);
        CHECK(CString::Strncmp(s + 0, s + 2, 2) < 0);
        CHECK(CString::Strncmp(s + 0, s + 2, 3) < 0);
        CHECK(CString::Strncmp(s + 2, s + 0, 2) > 0);
        CHECK(CString::Strncmp(s + 2, s + 0, 3) > 0);
    }

    SECTION("Strnlen") {
        CHECK(CString::Strnlen("", 0) == 0);
        CHECK(CString::Strnlen("", 1) == 0);
        CHECK(CString::Strnlen("", 2) == 0);
        CHECK(CString::Strnlen("a", 0) == 0);
        CHECK(CString::Strnlen("a", 1) == 1);
        CHECK(CString::Strnlen("a", 2) == 1);
        CHECK(CString::Strnlen("ab", 0) == 0);
        CHECK(CString::Strnlen("ab", 1) == 1);
        CHECK(CString::Strnlen("ab", 2) == 2);
    }

    SECTION("Strrchr") {
        char s[] = "xyxz";
        CHECK(CString::Strrchr(s + 0, '\0') == s + 4);
        CHECK(CString::Strrchr(s + 0, 'w') == nullptr);
        CHECK(CString::Strrchr(s + 0, 'x') == s + 2);
        CHECK(CString::Strrchr(s + 0, 'y') == s + 1);
        CHECK(CString::Strrchr(s + 0, 'z') == s + 3);
    }
}
