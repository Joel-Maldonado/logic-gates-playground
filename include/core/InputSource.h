#ifndef INPUT_SOURCE_H
#define INPUT_SOURCE_H

#include "core/LogicGate.h"
#include <string>

struct InputSourceColors {
    Color bodyOff;
    Color bodyOn;
    Color textColor;
    Color outlineColor;

    InputSourceColors(Color bOff = DARKGRAY, Color bOn = GREEN,
                      Color txt = WHITE, Color outl = BLACK)
        : bodyOff(bOff), bodyOn(bOn), textColor(txt), outlineColor(outl) {}
};

class InputSource : public LogicGate {
public:
    InputSource(std::string id, Vector2 pos, Vector2 size, std::string label);
    ~InputSource() override;

    void evaluate() override;
    void draw() override;

    void handleInput(Vector2 mousePos);
    void toggleState();
    void setState(bool newState);
    bool getCurrentState() const;

private:
    std::string label;
    bool internalState;
    InputSourceColors colors;
};

#endif // INPUT_SOURCE_H