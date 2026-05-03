#pragma once

#include <Arduino.h>

enum class Align : uint8_t {
    Left,   // value follows label; width not used
    Center, // valueX = x + (width / 2)
    Right,  // value right-aligned within width
    Spread  // label left edge, value right edge within width
};


class MenuItem {
public:
    virtual ~MenuItem() = default;

    bool isFocused() const { return focused; }
    virtual void setFocused(bool value) {
        focused = value;
        if (!value) {
            editing = false;
        }
    }

    bool isEditing() const { return editing; }
    void setEditing(bool value) { editing = value; }

    void setPosition(int16_t px, int16_t py) {
        x = px;
        y = py;
    }

    virtual void draw() = 0;

    virtual bool onEncoderTurn(int delta) { (void)delta; return false; }

    virtual void onButtonPress() {}

protected:
    MenuItem(int16_t px, int16_t py) : x(px), y(py), focused(false), editing(false) {}

    int16_t x;
    int16_t y;

private:
    bool focused;
    bool editing;
};
