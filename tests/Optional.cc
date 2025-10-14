#include <portable/Optional.hh>
#include <snitch/snitch_all.hpp>

#include <string>

TEST_CASE("Optional") {
    Optional<std::string> none;
    Optional<std::string> zero("0");
    Optional<std::string> one("1");

    SECTION("operator bool") {
        CHECK_FALSE(none);
        CHECK(zero);
    }

    SECTION("operator*") {
        CHECK(*zero == "0");
        *zero = *one;
        CHECK(*zero == "1");
    }

    SECTION("operator->") {
        CHECK(zero->at(0) == '0');
        zero->at(0) = one->at(0);
        CHECK(zero->at(0) == '1');
    }

    SECTION("get") {
        CHECK_FALSE(none.get());
        CHECK(*zero.get() == "0");
        *zero.get() = *one.get();
        CHECK(*zero.get() == "1");
    }

    SECTION("reset") {
        none.reset();
        CHECK_FALSE(none);
        zero.reset();
        CHECK_FALSE(zero);
    }

    SECTION("emplace") {
        auto &noneRef = none.emplace();
        CHECK(none == "");
        noneRef = "2";
        CHECK(none == "2");
        auto &zeroRef = zero.emplace();
        CHECK(zero == "");
        zeroRef = "3";
        CHECK(zero == "3");
    }

    SECTION("getOrEmplace") {
        auto &noneRef = none.getOrEmplace();
        CHECK(none == "");
        noneRef = "2";
        CHECK(none == "2");
        auto &zeroRef = zero.getOrEmplace();
        CHECK(zero == "0");
        zeroRef = "3";
        CHECK(zero == "3");
    }

    SECTION("operator==") {
        CHECK(none == none);
        CHECK_FALSE(none == zero);
        CHECK_FALSE(none == "0");
        CHECK(zero == zero);
        CHECK(zero == "0");
        CHECK_FALSE(zero == one);
        CHECK_FALSE(zero == "1");
    }

    SECTION("operator!=") {
        CHECK_FALSE(none != none);
        CHECK(none != zero);
        CHECK(none != "0");
        CHECK_FALSE(zero != zero);
        CHECK_FALSE(zero != "0");
        CHECK(zero != one);
        CHECK(zero != "1");
    }
}
