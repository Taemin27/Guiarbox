#pragma once


class Page {
public:
    virtual ~Page() {}

    virtual void setup() = 0;
    virtual void loop() = 0;

    virtual void update() {}

    bool isActive() {
        return active;
    }

    void setActive(bool active) {
        this -> active = active;
    }

    virtual void home() = 0;

private:
    bool active = false;
};


// temporary
class LegacyPage: public Page {
private :
    void (*_setupFunc)();
    void (*_loopFunc)();
public:
    LegacyPage(void (*s)(), void (*l)()) : _setupFunc(s), _loopFunc(l) {}    
    void setup() override { _setupFunc();}
    void loop() override { _loopFunc();}
    void home() override {}
};