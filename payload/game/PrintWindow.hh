#pragma once

#include <jsystem/J2DTextBox.hh>
#include <jsystem/JKRHeap.hh>

class PrintWindow {
public:
    PrintWindow(JKRHeap *heap);
    void draw();
    void calc();
    J2DTextBox *getTextBox() const;

private:
    u8 _000[0x2e8 - 0x000];

public:
    u32 m_size;
    u32 m_choiceType;
    u32 m_color;
    float m_frame;
    u32 m_choice;

private:
    u8 _2fc[0x308 - 0x2fc];
};
size_assert(PrintWindow, 0x308);
