#pragma once

#include <Arduino.h>
#include "../config/Colors.h"
#include "../config/Display.h"

extern GFXBuffer_t display;

class SystemMessage {
public:
    static void show(const char* msg, uint32_t durationMs,
                     void (*dismissCallback)() = nullptr, uint8_t textSize = 1) {
        if (!msg) {
            return;
        }
        strncpy(message, msg, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
        onDismiss = dismissCallback;
        messageTextSize = textSize < 1 ? 1 : textSize;
        dismissAt = millis() + durationMs;
        active = true;
        flushDisplay();
    }

    static void update() {
        if (!active) {
            return;
        }
        if (millis() >= dismissAt) {
            active = false;
            void (*callback)() = onDismiss;
            onDismiss = nullptr;
            if (callback) {
                callback();
            }
        }
    }

    static bool isVisible() {
        return active;
    }

    // Called from flushDisplay() so the overlay is composited once per blit.
    static void compositeIfVisible() {
        if (!active) {
            return;
        }
        drawOverlay();
    }

private:
    static constexpr int16_t MARGIN = 6;
    static constexpr int16_t PAD = 4;
    static constexpr uint16_t BOX_FILL = 0x4208;

    static inline bool active = false;
    static inline unsigned long dismissAt = 0;
    static inline void (*onDismiss)() = nullptr;
    static inline char message[128] = {};
    static inline uint8_t messageTextSize = 1;

    static int16_t boxX() {
        return MARGIN;
    }

    static int16_t boxY() {
        return MARGIN;
    }

    static int16_t boxW() {
        return display.width() - 2 * MARGIN;
    }

    static int16_t textInnerWidth() {
        return boxW() - 2 * PAD;
    }

    static int16_t lineHeight() {
        return 8 * messageTextSize;
    }

    static int16_t measureSubstringWidth(const char* start, int16_t len) {
        if (len <= 0) {
            return 0;
        }
        char buf[sizeof(message)];
        if (len >= (int16_t)sizeof(buf)) {
            len = sizeof(buf) - 1;
        }
        memcpy(buf, start, len);
        buf[len] = '\0';

        int16_t x1 = 0;
        int16_t y1 = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    static void printSubstring(const char* start, int16_t len, int16_t x, int16_t y) {
        if (len <= 0) {
            return;
        }
        char buf[sizeof(message)];
        if (len >= (int16_t)sizeof(buf)) {
            len = sizeof(buf) - 1;
        }
        memcpy(buf, start, len);
        buf[len] = '\0';
        display.setCursor(x, y);
        display.print(buf);
    }

    static int16_t layoutWrappedLines(bool draw, int16_t maxLines = INT16_MAX) {
        display.setTextSize(messageTextSize);
        const int16_t maxW = textInnerWidth();
        const int16_t lh = lineHeight();
        int16_t y = boxY() + PAD;
        int16_t lines = 0;
        const char* p = message;

        while (*p && lines < maxLines) {
            const char* lineStart = p;
            int16_t lineLen = 0;
            int16_t lastSpaceLen = 0;

            while (p[lineLen] != '\0' && p[lineLen] != '\n') {
                const int16_t tryLen = lineLen + 1;
                if (measureSubstringWidth(lineStart, tryLen) > maxW) {
                    if (lastSpaceLen > 0) {
                        lineLen = lastSpaceLen;
                    } else if (lineLen == 0) {
                        lineLen = 1;
                    }
                    break;
                }
                lineLen = tryLen;
                if (p[lineLen - 1] == ' ') {
                    lastSpaceLen = lineLen;
                }
            }

            int16_t drawLen = lineLen;
            while (drawLen > 0 && lineStart[drawLen - 1] == ' ') {
                drawLen--;
            }

            if (draw) {
                printSubstring(lineStart, drawLen, boxX() + PAD, y);
            }

            lines++;
            y += lh;

            p += lineLen;
            if (p[0] == '\n') {
                p++;
            }
            while (*p == ' ') {
                p++;
            }
        }

        return lines;
    }

    static void drawOverlay() {
        const int16_t lineCount = layoutWrappedLines(false);
        const int16_t maxBoxH = display.height() - 2 * MARGIN;
        int16_t boxH = 2 * PAD + lineCount * lineHeight();
        if (boxH > maxBoxH) {
            boxH = maxBoxH;
        }

        const int16_t x = boxX();
        const int16_t y = boxY();
        const int16_t w = boxW();

        display.fillRect(x, y, w, boxH, BOX_FILL);
        display.drawRect(x, y, w, boxH, WHITE);
        display.setTextWrap(false);
        display.setTextColor(WHITE, BOX_FILL);

        const int16_t maxLines = (boxH - 2 * PAD) / lineHeight();
        layoutWrappedLines(true, maxLines);
    }
};
