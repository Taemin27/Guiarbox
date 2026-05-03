#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"
#include <stdio.h>
#include <string.h>

class OptionsSelector : public MenuItem {
public:
    OptionsSelector(int16_t px, int16_t py, int16_t width, const char* label,
                    const char* const* options, int optionCount, int* selectedIndex,
                    uint8_t textSize = 2, Align align = Align::Spread)
        : MenuItem(px, py),
          width(width),
          label(label),
          options(options),
          optionCount(optionCount),
          selectedIndex(selectedIndex),
          textSize(textSize),
          align(align),
          optionFieldWidth(1) {
        if (options && optionCount > 0) {
            for (int i = 0; i < optionCount; i++) {
                int w = (int)strlen(options[i]);
                if (w > optionFieldWidth) {
                    optionFieldWidth = w;
                }
            }
            *selectedIndex = constrain(*selectedIndex, 0, optionCount - 1);
        }
    }

    void draw() override {
        const char* choice = "";
        if (options && optionCount > 0) {
            *selectedIndex = constrain(*selectedIndex, 0, optionCount - 1);
            choice = options[*selectedIndex];
        }

        char buf[48];
        if (align == Align::Left) {
            snprintf(buf, sizeof(buf), "%-*s", optionFieldWidth, choice);
        } else {
            snprintf(buf, sizeof(buf), "%*s", optionFieldWidth, choice);
        }

        display.setTextSize(textSize);
        display.setTextWrap(false);

        int16_t x1, y1;
        uint16_t tw, th;
        display.getTextBounds(buf, 0, 0, &x1, &y1, &tw, &th);

        int16_t lx1, ly1;
        uint16_t ltw, lth;
        display.getTextBounds(label, 0, 0, &lx1, &ly1, &ltw, &lth);

        int labelX = x;
        int valueX = x + (int)ltw;

        if (align == Align::Spread) {
            labelX = x;
            valueX = x + width - (int)tw;
        } else {
            const int groupW = (int)ltw + (int)tw;
            int groupX = x;
            if (align == Align::Right) {
                groupX = x + width - groupW;
            } else if (align == Align::Center) {
                groupX = x + (width / 2) - (groupW / 2);
            } else {
                // Left: group stays at x; width ignored by design.
                groupX = x;
            }
            labelX = groupX;
            valueX = groupX + (int)ltw;
        }

        display.setTextColor(WHITE, BLACK);
        display.setCursor(labelX, y);
        display.print(label);
        display.setCursor(valueX, y);
        display.print(buf);

        if (isFocused() && !isEditing()) {
            display.setTextColor(BLUE, BLACK);
            display.setCursor(labelX, y);
            display.print(label);
        }
        if (isFocused() && isEditing()) {
            display.setTextColor(BLUE, BLACK);
            display.setCursor(valueX, y);
            display.print(buf);
        }
    }

    bool onEncoderTurn(int delta) override {
        if (!isFocused() || !isEditing() || delta == 0 || optionCount <= 0) {
            return false;
        }
        *selectedIndex += delta;
        *selectedIndex = constrain(*selectedIndex, 0, optionCount - 1);
        return true;
    }

    void onButtonPress() override {
        if (!isFocused()) {
            return;
        }
        setEditing(!isEditing());
    }

private:
    int16_t width;
    const char* label;
    const char* const* options;
    int optionCount;
    int* selectedIndex;
    uint8_t textSize;
    Align align;
    int optionFieldWidth;
};
