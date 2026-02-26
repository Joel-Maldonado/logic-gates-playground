#include "core/GatePin.h"
#include "core/LogicGate.h"
#include "core/Wire.h"
#include "app/Config.h"
#include <raymath.h>
#include <algorithm>
#include <iostream>

GatePin::GatePin(LogicGate* parent, PinType pinType, int id, Vector2 offset)
    : parentGate_(parent),
      type_(pinType),
      pinId_(id),
      relativeOffset_(offset),
      clickRadius_(Config::PIN_CLICK_RADIUS),
      sourceOutputPin_(nullptr),
      currentState_(false) {
}

LogicGate* GatePin::getParentGate() const {
    return parentGate_;
}

PinType GatePin::getType() const {
    return type_;
}

int GatePin::getId() const {
    return pinId_;
}

Vector2 GatePin::getRelativeOffset() const {
    return relativeOffset_;
}

float GatePin::getClickRadius() const {
    return clickRadius_;
}

const GatePin* GatePin::getSourceOutputPin() const {
    return sourceOutputPin_;
}

const std::vector<GatePin*>& GatePin::getDependentInputPins() const {
    return dependentInputPins_;
}

bool GatePin::getState() const {
    if (type_ == PinType::INPUT_PIN) {
        if (sourceOutputPin_) {
            return sourceOutputPin_->getState();
        } else {
            return currentState_;
        }
    }
    return currentState_;
}

bool GatePin::setState(bool newState) {
    if (type_ == PinType::INPUT_PIN) {
        if (currentState_ != newState) {
            currentState_ = newState;
            if (parentGate_) {
                parentGate_->markDirty();
            }
            return true;
        }
        return false;
    }

    if (currentState_ != newState) {
        currentState_ = newState;
        for (GatePin* dependentPin : dependentInputPins_) {
            if (dependentPin && dependentPin->getParentGate()) {
                dependentPin->getParentGate()->markDirty();
            }
        }
        return true;
    }

    return false;
}

void GatePin::connectTo(GatePin* outputPin) {
    if (type_ != PinType::INPUT_PIN) {
        std::cerr << "Error: connectTo called on a non-INPUT_PIN." << std::endl;
        return;
    }
    if (outputPin == nullptr) {
        disconnectSource();
        if (parentGate_) parentGate_->markDirty();
        return;
    }
    if (outputPin->getType() != PinType::OUTPUT_PIN) {
        std::cerr << "Error: InputPin cannot connect to a non-OUTPUT_PIN." << std::endl;
        return;
    }
    if (sourceOutputPin_ == outputPin) {
        return;
    }

    if (sourceOutputPin_) {
        sourceOutputPin_->removeDependentPin(this);
    }

    sourceOutputPin_ = outputPin;
    sourceOutputPin_->addDependentPin(this);

    if (parentGate_) {
        parentGate_->markDirty();
    }
}

void GatePin::disconnectSource() {
    if (type_ == PinType::INPUT_PIN && sourceOutputPin_) {
        sourceOutputPin_->removeDependentPin(this);
        sourceOutputPin_ = nullptr;
        if (parentGate_) {
            parentGate_->markDirty();
        }
    }
}

void GatePin::addDependentPin(GatePin* inputPin) {
    if (type_ == PinType::OUTPUT_PIN && inputPin && inputPin->getType() == PinType::INPUT_PIN) {
        if (std::find(dependentInputPins_.begin(), dependentInputPins_.end(), inputPin) == dependentInputPins_.end()) {
            dependentInputPins_.push_back(inputPin);
        }
    }
}

void GatePin::removeDependentPin(GatePin* inputPin) {
    if (type_ == PinType::OUTPUT_PIN && inputPin) {
        dependentInputPins_.erase(
            std::remove(dependentInputPins_.begin(), dependentInputPins_.end(), inputPin),
            dependentInputPins_.end()
        );
    }
}

Vector2 GatePin::getAbsolutePosition() const {
    if (parentGate_) {
        Vector2 parentPos = parentGate_->getPosition();
        return { parentPos.x + relativeOffset_.x, parentPos.y + relativeOffset_.y };
    }
    return relativeOffset_;
}

bool GatePin::isConnectedInput() const {
    return (type_ == PinType::INPUT_PIN && sourceOutputPin_ != nullptr);
}

bool GatePin::hasConnectedOutputDependents() const {
    return (type_ == PinType::OUTPUT_PIN && !dependentInputPins_.empty());
}

bool GatePin::isConnected() const {
    return isConnectedInput() || hasConnectedOutputDependents();
}

bool GatePin::isMouseOverPin(Vector2 mousePos) const {
    if (!parentGate_) return false;
    return CheckCollisionPointCircle(mousePos, getAbsolutePosition(), clickRadius_);
}

void GatePin::disconnectWire(Wire* wireToDisconnect) {
    if (!wireToDisconnect) return;

    if (type_ == PinType::INPUT_PIN) {
        // This input pin is the destination of wireToDisconnect
        // If this wire is the one connected nullify the source
        if (sourceOutputPin_ && wireToDisconnect->getDestPin() == this &&
            wireToDisconnect->getSourcePin() == sourceOutputPin_) {
            sourceOutputPin_ = nullptr;
            if (parentGate_) {
                parentGate_->markDirty();
            }
        }
    } else { // PinType::OUTPUT_PIN
        // This output pin is the source of wireToDisconnect
        // Remove the wires destination pin from list of dependents
        if (wireToDisconnect->getSourcePin() == this) {
            GatePin* dependentDestPin = wireToDisconnect->getDestPin();
            if (dependentDestPin) {
                removeDependentPin(dependentDestPin);
            }
        }
    }
}
