#pragma once

#include "jsystem/J2DPane.hh"

class J2DTextBox : public J2DPane {
public:
    char *getString() const;

private:
    u8 _100[0x138 - 0x100];
};
size_assert(J2DTextBox, 0x138);
