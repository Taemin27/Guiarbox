#pragma once

#include "MenuItem.h"
#include "../config/Colors.h"
#include <vector>
#include <initializer_list>
#include <stddef.h>

extern GFXBuffer_t display;


class ScrollableMenuView : public MenuItem {
public:
    ScrollableMenuView(
        std::initializer_list<std::initializer_list<MenuItem*>> itemsInit)
        : MenuItem(0, 0), groupIndex(0), cursor(0) {
        items.reserve(itemsInit.size());
        for (const auto& groupInit : itemsInit) {
            std::vector<MenuItem*> group;
            group.reserve(groupInit.size());
            for (MenuItem* item : groupInit) {
                group.push_back(item);
            }
            items.push_back(std::move(group));
        }
    }

    ScrollableMenuView(std::vector<std::vector<MenuItem*>> itemsInit)
        : MenuItem(0, 0), items(std::move(itemsInit)), groupIndex(0), cursor(0) {}

    void draw() override {
        for (const auto& item : items[(size_t)groupIndex]) {
            item->draw();
        }
    }

    void onButtonPress() override {
        if (isFocused()) {
            currentItem()->onButtonPress();
        }
    }

    void setFocused(bool value) override {
        MenuItem::setFocused(value);

        if (!value) {
            setEditing(false);
            currentItem()->setFocused(false);
            return;
        }

        setEditing(true);
        currentItem()->setFocused(true);
    }

    bool onEncoderTurn(int delta) override {
        if (!isFocused()) {
            return false;
        }

        MenuItem* item = currentItem();
        if (item->onEncoderTurn(delta)) {
            return true;
        }

        const int nextCursor = cursor + delta;
        const int nextGroup = groupIndex + delta;

        if (nextCursor >= 0 && nextCursor < (int)items[(size_t)groupIndex].size()) {
            item->setFocused(false);
            cursor = nextCursor;
            items[(size_t)groupIndex][(size_t)cursor]->setFocused(true);
            return true;
        }

        if (nextGroup >= 0 && nextGroup < (int)items.size()) {
            item->setFocused(false);
            groupIndex = nextGroup;
            cursor = delta > 0 ? 0 : (int)items[(size_t)groupIndex].size() - 1;
            items[(size_t)groupIndex][(size_t)cursor]->setFocused(true);
            return true;
        }

        // At first/last item and page: do not unfocus the child — nothing consumes the turn but
        // the highlight should stay on the current parameter.
        return false;
    }

private:
    MenuItem* currentItem() {
        return items[(size_t)groupIndex][(size_t)cursor];
    }

    std::vector<std::vector<MenuItem*>> items;
    int groupIndex;
    int cursor;
};
