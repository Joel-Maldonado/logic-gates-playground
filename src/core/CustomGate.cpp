#include "core/CustomGate.h"
#include "core/GatePin.h" // For GatePin details if needed
#include "core/Wire.h"    // For creating internal wires
#include "core/DerivedGates.h" // For instantiating basic gates like AndGate, OrGate, etc.
#include "core/InputSource.h"  // Added as requested
#include "core/OutputSink.h"   // Added as requested
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For std::cerr

// Constructor
CustomGate::CustomGate(std::string gateId, Vector2 pos, const CustomGateData& definition)
    : LogicGate(gateId, pos, 100, 50 + (definition.numExternalInputPins > 0 ? definition.numExternalInputPins : 1) * 15 + (definition.numExternalOutputPins > 0 ? definition.numExternalOutputPins : 1) * 15), // Placeholder width/height, ensure minimum height
      definition_(definition),
      internalCircuit_(std::make_unique<CircuitSimulator>()), // Boolean argument removed
      isValid_(true) // Assume valid until an error occurs during setup
{
    if (definition.numExternalInputPins < 0) {
        std::cerr << "Error (CustomGate " << id_ << "): numExternalInputPins is negative (" << definition.numExternalInputPins << "). Setting to 0." << std::endl;
        definition_.numExternalInputPins = 0; // Correct data locally
    }
    if (definition.numExternalOutputPins < 0) {
        std::cerr << "Error (CustomGate " << id_ << "): numExternalOutputPins is negative (" << definition.numExternalOutputPins << "). Setting to 0." << std::endl;
        definition_.numExternalOutputPins = 0; // Correct data locally
    }

    // Initialize external pins based on (potentially corrected) definition
    // Inputs
    for (int i = 0; i < definition_.numExternalInputPins; ++i) {
        // Calculate pin position (example: left side)
        Vector2 pinPos = { 0, 20.0f + i * 20.0f }; // Relative to gate's position_
        initializeInputPin(i, pinPos);
    }

    // Outputs
    for (int i = 0; i < definition_.numExternalOutputPins; ++i) {
        // Calculate pin position (example: right side)
        Vector2 pinPos = { width_, 20.0f + i * 20.0f }; // Relative to gate's position_
        initializeOutputPin(i, pinPos);
    }

    // Setup the internal circuit
    setupInternalCircuit();

    if (!isValid_) {
        std::cerr << "Error (CustomGate " << id_ << "): Setup failed. Gate will be non-functional." << std::endl;
        // Clear any partially initialized pins from LogicGate base if setup failed midway
        inputPins_.clear();
        outputPins_.clear();
        // Optionally, resize to declared number of pins but leave them non-functional
        // Or, ensure LogicGate base class handles this gracefully if pins are not fully initialized.
    }
    // After setupInternalCircuit, inputPins_ and outputPins_ of LogicGate base are populated IF setup was valid.
    // The maps externalToInternalInputPinMap_ and internalToExternalOutputPinMap_ are also populated.
}

void CustomGate::setupInternalCircuit() {
    if (!internalCircuit_) {
        // This should ideally not happen if constructor logic is sound
        std::cerr << "Critical Error (CustomGate " << id_ << "): Internal circuit simulator is null." << std::endl;
        isValid_ = false;
        return;
    }

    // 1. Instantiate internal gates
    for (const auto& gateDesc : definition_.internalGates) {
        if (!isValid_) return; // Stop if a previous step failed

        LogicGate* newGate = nullptr;
        Vector2 internalGatePos = {gateDesc.posX, gateDesc.posY};

        // Simple factory logic (expand as needed)
        if (gateDesc.type == "AndGate") {
            newGate = new AndGate(gateDesc.id, internalGatePos);
        } else if (gateDesc.type == "OrGate") {
            newGate = new OrGate(gateDesc.id, internalGatePos);
        } else if (gateDesc.type == "NotGate") {
            newGate = new NotGate(gateDesc.id, internalGatePos);
        } else if (gateDesc.type == "InputSource") {
            newGate = new InputSource(gateDesc.id, internalGatePos, {20.0f, 20.0f}, gateDesc.id);
        } else if (gateDesc.type == "OutputSink") {
            newGate = new OutputSink(gateDesc.id, internalGatePos, 10.0f, gateDesc.id);
        }
        // TODO: Add support for "CustomGate:MyPreviousCustom" type
        else {
            std::cerr << "Error (CustomGate " << id_ << "): Unknown internal gate type '" << gateDesc.type << "' with id '" << gateDesc.id << "'." << std::endl;
            isValid_ = false;
            return; // Critical error, stop setup
        }

        if (newGate) {
            if (!internalCircuit_->addGate(std::unique_ptr<LogicGate>(newGate))) {
                 std::cerr << "Error (CustomGate " << id_ << "): Failed to add internal gate id '" << gateDesc.id << "' (duplicate id?) to internal circuit." << std::endl;
                 isValid_ = false;
                 // newGate is a raw pointer, unique_ptr constructor failed or addGate failed before taking ownership.
                 // if addGate takes unique_ptr by value and fails, it's deleted. If by const ref and fails, we might need to delete.
                 // Assuming addGate handles newGate if it fails internally, or doesn't take ownership on failure.
                 // For safety, if addGate returns false, we might leak newGate if it wasn't converted to unique_ptr yet.
                 // CircuitSimulator::addGate takes std::unique_ptr<LogicGate>, so newGate is owned by the unique_ptr passed.
                 // If CircuitSimulator::addGate returns false, the unique_ptr is destroyed, cleaning up newGate. So no leak here.
                 return;
            }
        } else {
            // Should not happen if types are handled above, but as a safeguard:
            std::cerr << "Error (CustomGate " << id_ << "): newGate was null after instantiation attempt for type '" << gateDesc.type << "'." << std::endl;
            isValid_ = false;
            return;
        }
    }

    // 2. Create internal wires
    for (const auto& wireDesc : definition_.internalWires) {
        if (!isValid_) return;

        LogicGate* fromGate = this->findInternalGateById(wireDesc.fromGateId);
        LogicGate* toGate = this->findInternalGateById(wireDesc.toGateId);

        if (!fromGate) {
            std::cerr << "Error (CustomGate " << id_ << "): Source gate for internal wire not found: '" << wireDesc.fromGateId << "'." << std::endl;
            isValid_ = false; return;
        }
        if (!toGate) {
            std::cerr << "Error (CustomGate " << id_ << "): Destination gate for internal wire not found: '" << wireDesc.toGateId << "'." << std::endl;
            isValid_ = false; return;
        }

        if (wireDesc.fromPinIndex < 0 || static_cast<size_t>(wireDesc.fromPinIndex) >= fromGate->getOutputPinCount()) {
            std::cerr << "Error (CustomGate " << id_ << "): Source pin index " << wireDesc.fromPinIndex << " out of bounds for gate '" << wireDesc.fromGateId << "' (outputs: " << fromGate->getOutputPinCount() << ")." << std::endl;
            isValid_ = false; return;
        }
        GatePin* fromPin = fromGate->getOutputPin(wireDesc.fromPinIndex);
        if (!fromPin || fromPin->getType() != PinType::OUTPUT_PIN) { // Second check is redundant if getOutputPin guarantees type
            std::cerr << "Error (CustomGate " << id_ << "): Source pin " << wireDesc.fromPinIndex << " on gate '" << wireDesc.fromGateId << "' is not a valid output pin." << std::endl;
            isValid_ = false; return;
        }

        if (wireDesc.toPinIndex < 0 || static_cast<size_t>(wireDesc.toPinIndex) >= toGate->getInputPinCount()) {
            std::cerr << "Error (CustomGate " << id_ << "): Destination pin index " << wireDesc.toPinIndex << " out of bounds for gate '" << wireDesc.toGateId << "' (inputs: " << toGate->getInputPinCount() << ")." << std::endl;
            isValid_ = false; return;
        }
        GatePin* toPin = toGate->getInputPin(wireDesc.toPinIndex);
         if (!toPin || toPin->getType() != PinType::INPUT_PIN) { // Second check is redundant
            std::cerr << "Error (CustomGate " << id_ << "): Destination pin " << wireDesc.toPinIndex << " on gate '" << wireDesc.toGateId << "' is not a valid input pin." << std::endl;
            isValid_ = false; return;
        }

        // std::string wireId = "internal_wire_" + wireDesc.fromGateId + "_" + std::to_string(wireDesc.fromPinIndex) + "_to_" + wireDesc.toGateId + "_" + std::to_string(wireDesc.toPinIndex); // Wire ID removed
        Wire* newInternalWire = internalCircuit_->createWire(fromPin, toPin); // Changed from addWire to createWire
        if (!newInternalWire) { // Check if the returned pointer is null
            std::cerr << "Error (CustomGate " << id_ << "): Failed to create internal wire from " << wireDesc.fromGateId << ":" << wireDesc.fromPinIndex
                      << " to " << wireDesc.toGateId << ":" << wireDesc.toPinIndex << " (e.g. input pin already connected, or createWire failed)." << std::endl;
            isValid_ = false; return;
        }
    }

    // 3. Populate pin mappings
    // Ensure external pin counts are non-negative (already checked in constructor, but vectors need valid sizes)
    if (definition_.numExternalInputPins > 0) {
        externalToInternalInputPinMap_.resize(definition_.numExternalInputPins, nullptr);
    }
    if (definition_.numExternalOutputPins > 0) {
        internalToExternalOutputPinMap_.resize(definition_.numExternalOutputPins, nullptr);
    }


    for (const auto& mapping : definition_.pinMappings) {
        if (!isValid_) return;

        LogicGate* internalGate = this->findInternalGateById(mapping.internalGateId);
        if (!internalGate) {
            std::cerr << "Error (CustomGate " << id_ << "): Gate for pin mapping not found: '" << mapping.internalGateId << "'." << std::endl;
            isValid_ = false; return;
        }

        if (mapping.isOutputMapping) { // Internal output to external output
            if (mapping.externalPinIndex < 0 || static_cast<size_t>(mapping.externalPinIndex) >= internalToExternalOutputPinMap_.size()) {
                 std::cerr << "Error (CustomGate " << id_ << "): External output pin index " << mapping.externalPinIndex << " out of range for pin mapping (max: " << internalToExternalOutputPinMap_.size() -1 << ")." << std::endl;
                 isValid_ = false; return;
            }
            if (mapping.internalPinIndex < 0 || static_cast<size_t>(mapping.internalPinIndex) >= internalGate->getOutputPinCount()) {
                std::cerr << "Error (CustomGate " << id_ << "): Internal output pin index " << mapping.internalPinIndex << " for mapping on gate '" << mapping.internalGateId << "' is out of bounds (outputs: " << internalGate->getOutputPinCount() << ")." << std::endl;
                isValid_ = false; return;
            }
            GatePin* internalPin = internalGate->getOutputPin(mapping.internalPinIndex);
            if (!internalPin || internalPin->getType() != PinType::OUTPUT_PIN) { // Redundant check
                std::cerr << "Error (CustomGate " << id_ << "): Pin " << mapping.internalPinIndex << " on gate '" << mapping.internalGateId << "' is not a valid output pin for output mapping." << std::endl;
                isValid_ = false; return;
            }
            if (internalToExternalOutputPinMap_[mapping.externalPinIndex] != nullptr) {
                 std::cerr << "Error (CustomGate " << id_ << "): External output pin " << mapping.externalPinIndex << " is already mapped." << std::endl;
                 isValid_ = false; return;
            }
            internalToExternalOutputPinMap_[mapping.externalPinIndex] = internalPin;
        } else { // External input to internal input
            if (mapping.externalPinIndex < 0 || static_cast<size_t>(mapping.externalPinIndex) >= externalToInternalInputPinMap_.size()) {
                 std::cerr << "Error (CustomGate " << id_ << "): External input pin index " << mapping.externalPinIndex << " out of range for pin mapping (max: " << externalToInternalInputPinMap_.size() -1 << ")." << std::endl;
                 isValid_ = false; return;
            }
            if (mapping.internalPinIndex < 0 || static_cast<size_t>(mapping.internalPinIndex) >= internalGate->getInputPinCount()) {
                std::cerr << "Error (CustomGate " << id_ << "): Internal input pin index " << mapping.internalPinIndex << " for mapping on gate '" << mapping.internalGateId << "' is out of bounds (inputs: " << internalGate->getInputPinCount() << ")." << std::endl;
                isValid_ = false; return;
            }
            GatePin* internalPin = internalGate->getInputPin(mapping.internalPinIndex);
            if (!internalPin || internalPin->getType() != PinType::INPUT_PIN) { // Redundant check
                 std::cerr << "Error (CustomGate " << id_ << "): Pin " << mapping.internalPinIndex << " on gate '" << mapping.internalGateId << "' is not a valid input pin for input mapping." << std::endl;
                 isValid_ = false; return;
            }
            // An external input can map to multiple internal input pins. The vector stores only one for primary mapping.
            // This logic might need refinement if one external input truly needs to drive multiple internal pins *through this map*.
            // The current design of `evaluate` uses this map to set state on ONE internal pin per external pin.
            // If multiple mappings exist for the same externalPinIndex, only the last one processed will be stored in externalToInternalInputPinMap_.
            // The XNOR test case correctly defines multiple PinMappingDesc for the same externalPinIndex.
            // This means externalToInternalInputPinMap_ isn't suitable for fan-out directly at the mapping stage.
            // The evaluate() loop needs to iterate all relevant mappings, not just this map.
            // For now, let's assume the map holds the "primary" or "first" mapping for simplicity of this error check phase.
            // A more robust solution would be `std::vector<std::vector<GatePin*>> externalToInternalInputPinMap_` or similar.
            // Or, the pinMappings vector itself is the source of truth for evaluation.
            // The current implementation of evaluate() iterates inputPins_ and uses externalToInternalInputPinMap_ - this implies one-to-one or one-to-primary.
            // This is a deeper design consideration than just error handling. For now, accept multiple mappings for the same external pin index in definition,
            // but the map will only store the last one. The XNOR test will reveal if this is an issue.
            // Given the XNOR test's pin mapping:
            // {0, "notA", 0, false} -> externalToInternalInputPinMap_[0] = notA->pin(0)
            // {0, "and_A_B", 0, false} -> externalToInternalInputPinMap_[0] = and_A_B->pin(0) (overwrites previous)
            // This needs to be fixed in how evaluate uses mappings.
            // For this error handling task, we ensure that *a* mapping is valid.
            // The check below for all mappings being populated for the *map* is also affected by this.
            if (externalToInternalInputPinMap_[mapping.externalPinIndex] != nullptr) {
                 // This is not an error if fan-out is allowed from an external pin.
                 // std::cout << "Warning (CustomGate " << id_ << "): External input pin " << mapping.externalPinIndex << " is being re-mapped. This is acceptable for fan-out." << std::endl;
            }
            externalToInternalInputPinMap_[mapping.externalPinIndex] = internalPin;
        }
    }

    // Verify all mappings for the *maps* are populated (considering the overwrite issue for inputs)
    for(size_t i = 0; i < externalToInternalInputPinMap_.size(); ++i) {
        if (!externalToInternalInputPinMap_[i]) {
            std::cerr << "Error (CustomGate " << id_ << "): External input pin " << i << " was not successfully mapped to any internal input pin through the primary map." << std::endl;
            isValid_ = false; // This might be too strict if some inputs are optional or fan-out is handled differently
            // For now, assume all declared external pins must have a valid primary mapping in the map.
        }
    }
    for(size_t i = 0; i < internalToExternalOutputPinMap_.size(); ++i) {
        if (!internalToExternalOutputPinMap_[i]) {
            std::cerr << "Error (CustomGate " << id_ << "): External output pin " << i << " was not successfully mapped from any internal output pin." << std::endl;
            isValid_ = false;
        }
    }
    if (!isValid_) return; // Stop if mapping checks failed
}

LogicGate* CustomGate::findInternalGateById(const std::string& id) {
    if (!internalCircuit_) {
        // This should ideally not happen if constructor is robust.
        // std::cerr << "Warning (CustomGate " << this->id_ << "): findInternalGateById called but internalCircuit_ is null." << std::endl;
        return nullptr;
    }
    // Assuming CircuitSimulator has getGates() returning a list of std::unique_ptr<LogicGate> or similar.
    // This is a linear scan, which is fine for moderate numbers of internal gates.
    // If CircuitSimulator offers a direct getGateById, that would be more efficient, but this helper
    // encapsulates the interaction with internalCircuit_ for finding gates.
    const auto& allInternalGates = internalCircuit_->getGates(); // Assumes CircuitSimulator::getGates() exists
    for (const auto& gate_ptr : allInternalGates) {
        if (gate_ptr && gate_ptr->getId() == id) {
            return gate_ptr.get();
        }
    }
    return nullptr;
}

void CustomGate::evaluate() {
    if (!isValid_ || !internalCircuit_) {
        // If not valid, or circuit doesn't exist, do nothing.
        // Output pins should retain their last state or be default (false).
        // LogicGate base class pins are initialized to false.
        isDirty_ = false; // Still mark as not needing evaluation.
        return;
    }

    // Propagate CustomGate's external input states to the mapped internal gates' input pins
    // THIS IS THE PART THAT NEEDS REVISION based on pinMappings structure.
    // Current logic: iterates CustomGate's inputPins_ and uses externalToInternalInputPinMap_
    // This only supports one internal pin being driven by each external pin via the map.
    // Correct logic: Iterate definition_.pinMappings for isOutputMapping == false
    for (const auto& mapping : definition_.pinMappings) {
        if (mapping.isOutputMapping == false) {
            if (mapping.externalPinIndex < 0 || static_cast<size_t>(mapping.externalPinIndex) >= inputPins_.size()) {
                 // Should have been caught in setup, but defensive check
                if (isValid_) { // Avoid spamming if already invalid
                    std::cerr << "Error (CustomGate " << id_ << " evaluate): Invalid externalPinIndex " << mapping.externalPinIndex << " in pinMappings." << std::endl;
                    // isValid_ = false; // Optionally re-flag
                }
                continue;
            }
            LogicGate* internalGate = this->findInternalGateById(mapping.internalGateId);
            GatePin* internalPin = nullptr;
            if (internalGate && mapping.internalPinIndex >= 0 && static_cast<size_t>(mapping.internalPinIndex) < internalGate->getInputPinCount()) {
                internalPin = internalGate->getInputPin(mapping.internalPinIndex);
            }

            if (internalPin) {
                internalPin->setState(inputPins_[mapping.externalPinIndex].getState());
            } else {
                 if (isValid_) {
                    std::cerr << "Error (CustomGate " << id_ << " evaluate): Invalid internal pin for mapping: ext pin " << mapping.externalPinIndex
                              << " to " << mapping.internalGateId << ":" << mapping.internalPinIndex << std::endl;
                 }
            }
        }
    }

    internalCircuit_->update(); // Changed from evaluateAll to update

    // Update CustomGate's external output pin states from the mapped internal gates' output pins
    for (size_t i = 0; i < outputPins_.size(); ++i) {
        if (i < internalToExternalOutputPinMap_.size() && internalToExternalOutputPinMap_[i]) {
            outputPins_[i].setState(internalToExternalOutputPinMap_[i]->getState());
        } else {
            // This implies not all external output pins were mapped, caught by setup checks if isValid_ is true.
            if (isValid_) {
                 std::cerr << "Error (CustomGate " << id_ << " evaluate): Unmapped or null internal pin for external output pin " << i << "." << std::endl;
                 // isValid_ = false; // Optionally re-flag
            }
        }
    }

    // Removed loop: for (auto& pin : outputPins_) { pin.propagateStateToConnectedWires(); }
    // The main simulator loop is expected to handle wire state propagation after all gates are evaluated.
    isDirty_ = false;
}

void CustomGate::draw() {
    if (!isValid_) {
        // Draw as an invalid/error box
        DrawRectangleV(position_, {width_, height_}, RED);
        DrawText("INVALID", position_.x + width_/2 - MeasureText("INVALID", 10)/2, position_.y + height_/2 - 5, 10, BLACK);
        DrawRectangleLinesEx(getBounds(), 1, BLACK); // Use getBounds for consistency
        // Still draw pins perhaps, but grayed out or with error indicators
        for (auto& pin : inputPins_) { // Use reference from LogicGate base
            Vector2 pinAbsPos = pin.getAbsolutePosition();
            DrawCircleV(pinAbsPos, pin.getClickRadius(), GRAY);
        }
        for (auto& pin : outputPins_) { // Use reference from LogicGate base
            Vector2 pinAbsPos = pin.getAbsolutePosition();
            DrawCircleV(pinAbsPos, pin.getClickRadius(), GRAY);
        }
        return;
    }

    // Basic drawing: A rectangle for the gate body
    DrawRectangleV(position_, {width_, height_}, LIGHTGRAY);
    DrawRectangleLinesEx(getBounds(), 1, (isSelected_ ? YELLOW : BLACK));
    DrawText(definition_.name.c_str(), position_.x + 10, position_.y + height_ / 2 - 10, 10, BLACK);

    // Draw external input pins (using the pins from LogicGate base class)
    for (const auto& pin : getAllInputPins()) { // Use const getter returning const ref
        Vector2 pinAbsPos = pin.getAbsolutePosition();
        DrawCircleV(pinAbsPos, pin.getClickRadius(), pin.getState() ? BLUE : DARKGRAY);
        if (pin.isHovered()) {
            DrawCircleLinesV(pinAbsPos, pin.getClickRadius() + 2, YELLOW);
        }
    }

    // Draw external output pins (using the pins from LogicGate base class)
    for (const auto& pin : getAllOutputPins()) { // Use const getter returning const ref
        Vector2 pinAbsPos = pin.getAbsolutePosition();
        DrawCircleV(pinAbsPos, pin.getClickRadius(), pin.getState() ? BLUE : DARKGRAY);
        if (pin.isHovered()) {
            DrawCircleLinesV(pinAbsPos, pin.getClickRadius() + 2, YELLOW);
        }
    }
    // For now, the custom gate is a black box.
}

// Optional overrides if LogicGate's implementation is not sufficient
// int CustomGate::getInputPinCount() const { return definition_.numExternalInputPins; }
// int CustomGate::getOutputPinCount() const { return definition_.numExternalOutputPins; }

// Note: Destructor not explicitly needed if std::unique_ptr handles internalCircuit_ cleanup,
// and LogicGate base class destructor handles its own resources.
// If internal gates or wires within internalCircuit_ require specific cleanup beyond what unique_ptr
// provides (e.g. raw pointers were used there), then a destructor would be needed.
// CircuitSimulator should handle deletion of its gates and wires.
