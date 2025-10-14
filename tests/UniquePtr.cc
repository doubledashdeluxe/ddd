#include <portable/UniquePtr.hh>
#include <snitch/snitch_all.hpp>

#include <string>

TEST_CASE("UniquePtr") {
    UniquePtr<std::string> none;
    UniquePtr<std::string> zero(new std::string("0"));
    UniquePtr<std::string> one(new std::string("1"));

    SECTION("operator bool") {
        CHECK_FALSE(none);
        CHECK(zero);
    }

    SECTION("operator*") {
        CHECK(*zero == "0");
    }

    SECTION("operator->") {
        CHECK(zero->at(0) == '0');
    }

    SECTION("get") {
        CHECK_FALSE(none.get());
        CHECK(*zero.get() == "0");
    }

    SECTION("release") {
        auto *nonePtr = none.release();
        CHECK_FALSE(nonePtr);
        auto *zeroPtr = zero.release();
        CHECK(*zeroPtr == "0");
        delete zeroPtr;
    }

    SECTION("reset") {
        none.reset();
        CHECK_FALSE(none);
        none.reset(new std::string("2"));
        CHECK(*none == "2");
        zero.reset();
        CHECK_FALSE(zero);
        one.reset(new std::string("3"));
        CHECK(*one == "3");
    }

    SECTION("Swap") {
        Swap(zero, one);
        CHECK(*zero == "1");
        CHECK(*one == "0");
    }
}
