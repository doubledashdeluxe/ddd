#include <portable/CardPath.hh>
#include <snitch/snitch_all.hpp>

#include <cstring>
#include <string>

TEST_CASE("CardPath") {
    using std::string_literals::operator""s;

    const char *dirPath = "main:/ddd/card a";

    char gameName[4];
    char company[2];
    memcpy(gameName, "GM4P", sizeof(gameName));
    memcpy(company, "01", sizeof(company));

    SECTION("SystemFile") {
        char fileName[32] = "MarioKart Double Dash!!";
        auto path = CardPath::Get(dirPath, fileName, gameName, company);
        CHECK(std::string(path.values()) == "main:/ddd/card a/01-GM4P-MarioKart Double Dash!!.gci");
    }

    SECTION("GhostFile") {
        char fileName[32] = "5!R)0*5&\"11!!!C##-=0_V%>*2%I!aa";
        auto path = CardPath::Get(dirPath, fileName, gameName, company);
        CHECK(std::string(path.values()) ==
                "main:/ddd/card a/01-GM4P-25-2m19s394-AAA-03-12-0f-aa.gci");
    }
}
