#ifndef INPUT_SOURCE_H
#define INPUT_SOURCE_H

#include "core/LogicGate.h"
#include <string>

/** Color configuration for input sources */
struct InputSourceColors {
    Color bodyOff;
    Color bodyOn;
    Color textColor;
    Color outlineColor;

    InputSourceColors(Color bOff = DARKGRAY, Color bOn = GREEN,
                      Color txt = WHITE, Color outl = BLACK)
        : bodyOff(bOff), bodyOn(bOn), textColor(txt), outlineColor(outl) {}
};

/**
 * Input source component that generates logic signals.
 * Can be toggled by user interaction to provide input to circuits.
 */
class InputSource : public LogicGate {
public:
    /**
     * Constructs an input source.
     * @param id Unique identifier
     * @param pos Position in world coordinates
     * @param size Dimensions of the component
     * @param label Display label
     */
    InputSource(std::string id, Vector2 pos, Vector2 size, std::string label);
    ~InputSource() override;

    void evaluate() override;
    void draw() override;

    // User interaction
    void handleInput(Vector2 mousePos);
    void toggleState();
    void setState(bool newState);
    bool getCurrentState() const;

private:
    std::string label_;
    bool internalState_;
    InputSourceColors colors_;
};

#endif // INPUT_SOURCE_H