#include "Kart.hh"

bool operator==(const Player &a, const Player &b) {
    return a.index == b.index && a.name == b.name;
}

bool operator!=(const Player &a, const Player &b) {
    return !(a == b);
}

bool operator==(const Kart &a, const Kart &b) {
    return a.local == b.local && a.playerCount == b.playerCount && a.players == b.players;
}

bool operator!=(const Kart &a, const Kart &b) {
    return !(a == b);
}
