#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"
#include <stdio.h>
#include <string.h>

class IntEditor : public MenuItem {
public:
    IntEditor(int16_t px, int16_t py, int16_t width, const char* label, int* value, int step = 1,
              int minValue = -2147483647, int maxValue = 2147483647, uint8_t textSize = 2,
              Align align = Align::Spread)
        : MenuItem(px, py),
          width(width),
          label(label),
          value(value),
          step(step),
          minValue(minValue),
          maxValue(maxValue),
          textSize(textSize),
          align(align) {
        char ta[16], tb[16];
        snprintf(ta, sizeof(ta), "%d", minValue);
        snprintf(tb, sizeof(tb), "%d", maxValue);
        int wa = (int)strlen(ta);
        int wb = (int)strlen(tb);
        valueFieldWidth = wa > wb ? wa : wb;
    }

    void draw() override {
        char buf[16];
        if (align == Align::Left) {
            snprintf(buf, sizeof(buf), "%-*d", valueFieldWidth, *value);
        } else {
            snprintf(buf, sizeof(buf), "%*d", valueFieldWidth, *value);
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
        if (!isFocused() || !isEditing() || delta == 0) {
            return false;
        }
        *value += step * delta;
        *value = constrain(*value, minValue, maxValue);
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
    int* value;
    int step;
    int minValue;
    int maxValue;
    uint8_t textSize;
    int valueFieldWidth;
    Align align;
};
