#pragma once

#include <common/Types.hh>

class Console {
public:
    static void Init(u8 bg, u8 fg);
    static void Print(const char *s);
    static void Print(u32 val);

private:
    Console();

    static void Print(char c);

    static u8 s_bg;
    static u8 s_fg;
    static u8 s_cols;
    static u8 s_rows;
    static u8 s_col;
    static u8 s_row;
};
