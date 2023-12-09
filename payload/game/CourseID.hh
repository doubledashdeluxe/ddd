#pragma once

class CourseID {
public:
    enum {
        BabyLuigi = 0x21, // Baby Park
        Peach = 0x22,     // Peach Beach
        Daisy = 0x23,     // Daisy Cruiser
        Luigi = 0x24,     // Luigi Circuit
        Mario = 0x25,     // Mario Circuit
        Yoshi = 0x26,     // Yoshi Circuit
        Nokonoko = 0x28,  // Mushroom Bridge
        Patapata = 0x29,  // Mushroom City
        Waluigi = 0x2a,   // Waluigi Stadium
        Wario = 0x2b,     // Wario Colosseum
        Diddy = 0x2c,     // Dino Dino Jungle
        Donkey = 0x2d,    // DK Mountain
        Koopa = 0x2f,     // Bowser's Castle
        Rainbow = 0x31,   // Rainbow Road
        Desert = 0x32,    // Dry Dry Desert
        Snow = 0x33,      // Sherbet Land
        Mini1 = 0x34,     // Luigi's Mansion
        Mini2 = 0x35,     // Nintendo GameCube
        Mini3 = 0x36,     // Block City
        Mini5 = 0x38,     // Tilt-a-Kart
        Mini7 = 0x3a,     // Cookie Land
        Mini8 = 0x3b,     // Pipe Plaza
        Award = 0x44,     // Award ceremony
        Ending = 0x45,    // Ending credits
    };

private:
    CourseID();
};
