#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"

class ToggleButton : public MenuItem {
public:
    ToggleButton(int16_t px, int16_t py, int16_t width, const char* label, bool* value,
                 uint8_t textSize = 2, Align align = Align::Left)
        : MenuItem(px, py),
          label(label),
          value(value),
          textSize(textSize),
          width(width),
          align(align) {}

    void draw() override {
        display.setTextSize(textSize);
        display.setTextWrap(false);

        uint16_t fg = (*value) ? GREEN : GRAY;
        uint16_t bg = isFocused() ? WHITE : BLACK;
        display.setTextColor(fg, bg);

        int labelX = x;
        if (align != Align::Left) {
            int16_t x1, y1;
            uint16_t tw, th;
            display.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
            if (align == Align::Right) {
                labelX = x + width - (int)tw;
            } else {
                // Center and Spread behave the same for a single label.
                labelX = x + (width / 2) - ((int)tw / 2);
            }
        }

        display.setCursor(labelX, y);
        display.print(label);
    }

    void onButtonPress() override {
        if (isFocused() && value) {
            *value = !(*value);
        }
    }

private:
    const char* label;
    bool* value;
    uint8_t textSize;
    int16_t width;
    Align align;
};
