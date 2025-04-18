#include <portable/UTF8.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("UTF8") {
    SECTION("ToAscii") {
        CHECK(UTF8::ToAscii(L'E') == 'E');
        CHECK(UTF8::ToAscii(L'É') == 'E');
        CHECK(UTF8::ToAscii(L'e') == 'e');
        CHECK(UTF8::ToAscii(L'é') == 'e');
    }

    SECTION("ToLower") {
        CHECK(UTF8::ToLower(L'E') == L'e');
        CHECK(UTF8::ToLower(L'É') == L'é');
        CHECK(UTF8::ToLower(L'e') == L'e');
        CHECK(UTF8::ToLower(L'é') == L'é');
    }

    SECTION("Length") {
        CHECK(UTF8::Length("rocket") == 6);
        CHECK(UTF8::Length("fusée") == 5);
    }

    SECTION("Next") {
        const char *s = "fusée";
        CHECK(UTF8::Next(s) == L'f');
        CHECK(UTF8::Next(s) == L'u');
        CHECK(UTF8::Next(s) == L's');
        CHECK(UTF8::Next(s) == L'é');
        CHECK(UTF8::Next(s) == L'e');
        CHECK(UTF8::Next(s) == L'\0');
    }

    SECTION("Compare") {
        CHECK(UTF8::Compare("Ou", "Ou") == 0);
        CHECK(UTF8::Compare("Ou", "Où") != 0);
        CHECK(UTF8::Compare("Ou", "ou") != 0);
        CHECK(UTF8::Compare("Ou", "où") != 0);
    }

    SECTION("CaseCompare") {
        CHECK(UTF8::CaseCompare("Ou", "Ou") == 0);
        CHECK(UTF8::CaseCompare("Ou", "Où") != 0);
        CHECK(UTF8::CaseCompare("Ou", "ou") == 0);
        CHECK(UTF8::CaseCompare("Ou", "où") != 0);
    }
}
