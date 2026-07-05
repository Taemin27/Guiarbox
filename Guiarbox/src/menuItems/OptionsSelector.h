#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"
#include "../config/Display.h"
#include "../config/Effects/EffectManager.h"
#include <stdio.h>
#include <string.h>

extern GFXBuffer_t display;

class OptionsSelector : public MenuItem {
public:
    // Static option list (RecordPage, BackingTrackPage, fixed effect enums).
    OptionsSelector(int16_t px, int16_t py, int16_t width, const char* label,
                    const char* const* options, int optionCount, int* selectedIndex,
                    uint8_t textSize = 2, Align align = Align::Spread)
        : MenuItem(px, py),
          width(width),
          label(label),
          bitmap(nullptr),
          bitmapW(0),
          bitmapH(0),
          liveParam(nullptr),
          options(options),
          optionCount(optionCount),
          selectedIndex(selectedIndex),
          textSize(textSize),
          align(align),
          optionFieldWidth(1) {
        initStaticOptionFieldWidth();
    }

    // Reads options/optionCount from EffectParameter on each draw/input (SD-backed lists).
    OptionsSelector(int16_t px, int16_t py, int16_t width, const char* label,
                    const EffectParameter* liveParam, int* selectedIndex,
                    uint8_t textSize = 2, Align align = Align::Spread)
        : MenuItem(px, py),
          width(width),
          label(label),
          bitmap(nullptr),
          bitmapW(0),
          bitmapH(0),
          liveParam(liveParam),
          options(nullptr),
          optionCount(0),
          selectedIndex(selectedIndex),
          textSize(textSize),
          align(align),
          optionFieldWidth(1) {
        clampSelectedIndex();
    }

    OptionsSelector(int16_t px, int16_t py, int16_t width, const uint8_t* bitmap, int16_t bitmapW, int16_t bitmapH,
                    const char* const* options, int optionCount, int* selectedIndex,
                    uint8_t textSize = 2, Align align = Align::Spread)
        : MenuItem(px, py),
          width(width),
          label(nullptr),
          bitmap(bitmap),
          bitmapW(bitmapW),
          bitmapH(bitmapH),
          liveParam(nullptr),
          options(options),
          optionCount(optionCount),
          selectedIndex(selectedIndex),
          textSize(textSize),
          align(align),
          optionFieldWidth(1) {
        initStaticOptionFieldWidth();
    }

    void setFocused(bool value) override {
        MenuItem::setFocused(value);
        if (!value) {
            resetScroll(false);
        }
    }

    bool tick() override {
        if (!isFocused() || !isEditing() || scrollPeriod <= 0) {
            return false;
        }
        return advanceScroll();
    }

    void draw() override {
        const char* const* activeOptions = resolveOptions();
        const int activeCount = resolveOptionCount();
        clampSelectedIndex();

        const char* choice = "";
        if (activeOptions && activeCount > 0) {
            choice = activeOptions[*selectedIndex];
            if (*selectedIndex != lastScrollIndex) {
                lastScrollIndex = *selectedIndex;
                resetScroll(isEditing());
            }
        }

        display.setTextSize(textSize);
        display.setTextWrap(false);

        int labelW = 0;
        if (bitmap) {
            labelW = bitmapW;
        } else if (label) {
            int16_t lx1, ly1;
            uint16_t ltw, lth;
            display.getTextBounds(label, 0, 0, &lx1, &ly1, &ltw, &lth);
            labelW = (int)ltw;
        }

        char buf[48];
        const char* valueText = choice;
        if (align == Align::Spread) {
            valueText = choice;
        } else if (align == Align::Left) {
            snprintf(buf, sizeof(buf), "%-*s", resolveOptionFieldWidth(), choice);
            valueText = buf;
        } else {
            snprintf(buf, sizeof(buf), "%*s", resolveOptionFieldWidth(), choice);
            valueText = buf;
        }

        int16_t x1, y1;
        uint16_t tw, th;
        display.getTextBounds(valueText, 0, 0, &x1, &y1, &tw, &th);

        const int valueMargin = 2;
        int valueAreaX = x + labelW + valueMargin;
        int valueAreaW = width - labelW - valueMargin;
        if (align != Align::Spread) {
            valueAreaX = x + labelW;
            valueAreaW = width - labelW;
        }
        if (valueAreaW < 8) {
            valueAreaX = x;
            valueAreaW = width;
        }

        recomputeScrollPeriod(valueText, valueAreaW);

        int labelX = x;
        int valueX = valueAreaX;
        int valueY = y;
        if (bitmap) {
            valueY = y + (bitmapH - (int)th) / 2 - (int)y1;
        }

        if (align == Align::Spread && scrollPeriod <= 0) {
            valueX = x + width - (int)tw;
            if (valueX < valueAreaX) {
                valueX = valueAreaX;
            }
        } else if (align != Align::Spread) {
            const int groupW = labelW + (int)tw;
            int groupX = x;
            if (align == Align::Right) {
                groupX = x + width - groupW;
            } else if (align == Align::Center) {
                groupX = x + (width / 2) - (groupW / 2);
            }
            labelX = groupX;
            valueX = groupX + labelW;
            valueAreaX = valueX;
            valueAreaW = width - (valueAreaX - x);
        }

        const uint16_t valueColor = (isFocused() && isEditing()) ? BLUE : WHITE;
        display.setTextColor(valueColor, BLACK);

        const int clipH = max((int)th, 16);

        if (scrollPeriod > 0 && isFocused() && isEditing()) {
            display.fillRect(valueAreaX, y, valueAreaW, clipH, BLACK);
            const int drawX = valueAreaX - scrollOffsetPx;
            display.setCursor(drawX, valueY);
            display.print(valueText);
            display.setCursor(drawX + scrollPeriod, valueY);
            display.print(valueText);
        } else if ((int)tw > valueAreaW) {
            char clipped[48];
            clipTextToWidth(valueText, valueAreaW, clipped, sizeof(clipped));
            if (align == Align::Spread) {
                display.getTextBounds(clipped, 0, 0, &x1, &y1, &tw, &th);
                valueX = x + width - (int)tw;
                if (valueX < valueAreaX) {
                    valueX = valueAreaX;
                }
            }
            display.setCursor(valueX, valueY);
            display.print(clipped);
        } else {
            display.setCursor(valueX, valueY);
            display.print(valueText);
        }

        if (scrollPeriod > 0 && isFocused() && isEditing() && valueAreaX > labelX) {
            display.fillRect(labelX, y, valueAreaX - labelX, clipH, BLACK);
        }

        if (bitmap) {
            display.drawBitmap(labelX, y, bitmap, bitmapW, bitmapH,
                               (isFocused() && !isEditing()) ? BLUE : WHITE);
        } else if (label) {
            display.setTextColor(WHITE, BLACK);
            display.setCursor(labelX, y);
            display.print(label);
            if (isFocused() && !isEditing()) {
                display.setTextColor(BLUE, BLACK);
                display.setCursor(labelX, y);
                display.print(label);
            }
        }
    }

    bool onEncoderTurn(int delta) override {
        if (!isFocused() || !isEditing() || delta == 0 || resolveOptionCount() <= 0) {
            return false;
        }
        *selectedIndex += delta;
        clampSelectedIndex();
        resetScroll(true);
        return true;
    }

    void onButtonPress() override {
        if (!isFocused()) {
            return;
        }
        const bool wasEditing = isEditing();
        setEditing(!wasEditing);
        if (!wasEditing && isEditing()) {
            resetScroll(true);
        } else if (wasEditing && !isEditing()) {
            resetScroll(false);
        }
    }

private:
    void initStaticOptionFieldWidth() {
        const char* const* activeOptions = resolveOptions();
        const int activeCount = resolveOptionCount();
        optionFieldWidth = 1;
        if (activeOptions && activeCount > 0) {
            for (int i = 0; i < activeCount; i++) {
                const int w = (int)strlen(activeOptions[i]);
                if (w > optionFieldWidth) {
                    optionFieldWidth = w;
                }
            }
            clampSelectedIndex();
        }
    }

    const char* const* resolveOptions() const {
        if (liveParam && liveParam->options && liveParam->optionCount > 0) {
            return liveParam->options;
        }
        return options;
    }

    int resolveOptionCount() const {
        if (liveParam && liveParam->optionCount > 0) {
            return liveParam->optionCount;
        }
        return optionCount;
    }

    int resolveMaxIndex() const {
        const int count = resolveOptionCount();
        return count > 0 ? count - 1 : 0;
    }

    int resolveOptionFieldWidth() const {
        if (!liveParam) {
            return optionFieldWidth;
        }
        const char* const* activeOptions = resolveOptions();
        const int activeCount = resolveOptionCount();
        int maxWidth = 1;
        if (activeOptions && activeCount > 0) {
            for (int i = 0; i < activeCount; i++) {
                const int w = (int)strlen(activeOptions[i]);
                if (w > maxWidth) {
                    maxWidth = w;
                }
            }
        }
        return maxWidth;
    }

    void clampSelectedIndex() {
        if (!selectedIndex) {
            return;
        }
        *selectedIndex = constrain(*selectedIndex, 0, resolveMaxIndex());
    }

    void resetScroll(bool pauseAtStart = false) {
        scrollOffsetPx = 0;
        scrollPausing = pauseAtStart;
        scrollLastStepMillis = millis();
        if (pauseAtStart) {
            scrollPauseStart = scrollLastStepMillis;
        }
    }

    void recomputeScrollPeriod(const char* text, int valueAreaW) {
        scrollPeriod = 0;
        if (!text || valueAreaW <= 0) {
            return;
        }

        int16_t x1, y1;
        uint16_t tw, th;
        display.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
        if ((int)tw > valueAreaW) {
            scrollPeriod = (int)tw + 48;
        }
        if (scrollPeriod > 0 && scrollOffsetPx >= scrollPeriod) {
            scrollOffsetPx = 0;
        }
    }

    bool advanceScroll() {
        if (scrollPeriod <= 0) {
            return false;
        }

        const unsigned long now = millis();
        if (scrollPausing) {
            if (now - scrollPauseStart >= 1500) {
                scrollPausing = false;
            }
            return false;
        }
        if (now - scrollLastStepMillis < 50) {
            return false;
        }

        scrollLastStepMillis = now;
        if (++scrollOffsetPx >= scrollPeriod) {
            scrollOffsetPx = 0;
            scrollPausing = true;
            scrollPauseStart = now;
        }
        return true;
    }

    static void clipTextToWidth(const char* text, int maxPx, char* out, size_t outSize) {
        if (!text || !out || outSize == 0) {
            if (out && outSize > 0) {
                out[0] = '\0';
            }
            return;
        }

        int16_t x1, y1;
        uint16_t tw, th;
        display.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
        if ((int)tw <= maxPx) {
            strncpy(out, text, outSize - 1);
            out[outSize - 1] = '\0';
            return;
        }

        const char* ellipsis = "...";
        size_t len = strlen(text);
        while (len > 0) {
            size_t copyLen = min(len, outSize - 4);
            memcpy(out, text, copyLen);
            out[copyLen] = '\0';
            strncat(out, ellipsis, outSize - copyLen - 1);
            display.getTextBounds(out, 0, 0, &x1, &y1, &tw, &th);
            if ((int)tw <= maxPx) {
                return;
            }
            len--;
        }

        strncpy(out, ellipsis, outSize - 1);
        out[outSize - 1] = '\0';
    }

    int16_t width;
    const char* label;
    const uint8_t* bitmap;
    int16_t bitmapW;
    int16_t bitmapH;
    const EffectParameter* liveParam;
    const char* const* options;
    int optionCount;
    int* selectedIndex;
    uint8_t textSize;
    Align align;
    int optionFieldWidth;

    int scrollOffsetPx = 0;
    int scrollPeriod = 0;
    bool scrollPausing = false;
    unsigned long scrollPauseStart = 0;
    unsigned long scrollLastStepMillis = 0;
    int lastScrollIndex = -1;
};
