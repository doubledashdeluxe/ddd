#pragma once

#include "portable/Array.hh"

#include <formats/Online.hh>

struct Player {
    u8 index;
    Array<char, PlayerNameLength + 1> name;
};

struct Kart {
    bool local;
    u8 playerCount;
    Array<Player, MaxKartPlayerCount> players;
    u16 mmr;
    u16 points;
};

bool operator==(const Player &a, const Player &b);
bool operator!=(const Player &a, const Player &b);

bool operator==(const Kart &a, const Kart &b);
bool operator!=(const Kart &a, const Kart &b);
