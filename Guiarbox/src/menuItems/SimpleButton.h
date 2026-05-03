#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"
#include <functional>

class SimpleButton : public MenuItem {
public:
    using Callback = std::function<void()>;

    SimpleButton(int16_t px, int16_t py, int16_t width, const char* label, Callback callback = nullptr,
                 uint8_t textSize = 2, Align align = Align::Left)
        : MenuItem(px, py),
          label(label),
          onPress(std::move(callback)),
          textSize(textSize),
          width(width),
          align(align) {}

    void draw() override {
        display.setTextSize(textSize);
        display.setTextWrap(false);
        display.setTextColor(WHITE, BLACK);

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
        if (isFocused()) {
            display.setTextColor(BLUE, BLACK);
            display.setCursor(labelX, y);
            display.print(label);
        }
    }

    void onButtonPress() override {
        if (isFocused() && onPress) {
            onPress();
        }
    }

private:
    const char* label;
    Callback onPress;
    uint8_t textSize;
    int16_t width;
    Align align;
};
