#include "core/InputSource.h"
#include <utility>

InputSource::InputSource(std::string id, Vector2 pos, Vector2 visualSize, std::string visualLabel)
    : LogicGate(std::move(id), GateKind::INPUT_SOURCE, pos, visualSize.x, visualSize.y),
      internalState_(false) {
    (void)visualLabel;
    initializeOutputPin(0, {visualSize.x, visualSize.y / 2.0f});

    markDirty();
}

InputSource::~InputSource() = default;

void InputSource::evaluate() {
    if (getOutputPinCount() > 0) {
        getOutputPin(0)->setState(internalState_);
    }
}

void InputSource::handleInput(Vector2 mousePos) {
    if (isMouseOver(mousePos) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        toggleState();
    }
}

void InputSource::toggleState() {
    internalState_ = !internalState_;
    markDirty();
}

bool InputSource::getCurrentState() const {
    return internalState_;
}

void InputSource::setState(bool newState) {
    if (internalState_ != newState) {
        internalState_ = newState;
        markDirty();
    }
}
