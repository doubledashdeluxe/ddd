#include <portable/Formatter.hh>
#include <snitch/snitch_all.hpp>

#include <iterator>
#include <string>

TEST_CASE("Formatter") {
    using std::string_literals::operator""s;

    char s[8];
    Formatter formatter(s, std::size(s));
    CHECK(s == ""s);
    formatter.printf("thunderstorm");
    CHECK(s == "thunder"s);
}
