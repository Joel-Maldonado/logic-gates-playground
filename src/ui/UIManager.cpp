#include "ui/UIManager.h"
#include "app/Config.h"
// core/CustomGateData.h is included via UIManager.h
#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <cstring> // For strcpy
#include <string>  // For std::to_string
#include <iostream> // For debugging output
#include <vector> // For std::vector in pin mapping removal
#include "core/CustomGateRegistry.h"

UIManager::UIManager(std::shared_ptr<CircuitSimulator> sim, CustomGateRegistry* registry)
    : isDrawingWire_(false),
      wireStartPin_(nullptr),
      isPanning_(false),
      panStartPosition_({0, 0}),
      lastMousePosition_({0, 0}),
      panVelocity_({0, 0}),
      panInertia_(0.9f),
      simulator_(sim),
      selectedComponent_(nullptr),
      selectedWire_(nullptr),
      isDraggingComponent_(false),
      isDraggingWirePoint_(false),
      dragStartOffset_({0, 0}),
      clickedInputSource_(nullptr),
      dragStartPosition_({0, 0}),
      currentUIMode_(UIMode::CIRCUIT_VIEW),
      editorNumInputs_(2), editorNumOutputs_(1),
      activeTextField_(0),
      customGateEditorPalette_(nullptr),
      selectedInternalGate_(nullptr),
      isDraggingInternalGate_(false),
      isDraggingFromEditorPalette_(false),
      isDrawingWireInEditor_(false),
      editorWireStartPin_(nullptr),
      mappingSelectedExtPinIndex_(-1),
      mappingSelectedExtPinType_(PinType::INPUT_PIN),
      isSelectingTargetForMapping_(false),
      customGateRegistry_(registry),
      editorStatusMessage_("") {

    strcpy(editorGateNameBuffer_, "MyCustomGate");
    strcpy(editorCategoryBuffer_, "Custom");

    camera_.target = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera_.offset = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;

    canvasBounds_ = {
        Config::PALETTE_WIDTH,
        0,
        (float)Config::SCREEN_WIDTH - Config::PALETTE_WIDTH,
        (float)Config::SCREEN_HEIGHT
    };

    paletteManager_ = std::make_unique<PaletteManager>(simulator_, customGateRegistry_); // Pass registry
    gateRenderer_ = std::make_unique<GateRenderer>();
    wireRenderer_ = std::make_unique<WireRenderer>();
}

void UIManager::initialize() {
    paletteManager_->initialize();
}

namespace EditorUI {
    Rectangle nameInputBounds;
    Rectangle categoryInputBounds;
    Rectangle inputIncBounds, inputDecBounds;
    Rectangle outputIncBounds, outputDecBounds;
    Rectangle saveButtonBounds, cancelButtonBounds;
    std::vector<Rectangle> extInputPinBounds;
    std::vector<Rectangle> extOutputPinBounds;
}

void UIManager::renderCustomGateEditorInterface() {
    float sidebarWidth = 250.0f;
    float padding = 10.0f;
    float currentY = padding;
    float itemHeight = 30.0f;
    float labelWidth = 80.0f;
    float fieldWidth = sidebarWidth - labelWidth - 3 * padding;
    float buttonWidth = itemHeight;

    DrawRectangle(0, 0, sidebarWidth, GetScreenHeight(), Fade(Config::Colors::PALETTE_VARIANT, 0.97f));
    DrawLine(sidebarWidth, 0, sidebarWidth, GetScreenHeight(), Config::Colors::GRID_LINE);

    DrawText("Gate Properties", padding, currentY + 5, 20, Config::Colors::PALETTE_TEXT);
    currentY += 30 + padding;

    DrawText("Name:", padding, currentY + (itemHeight - 20)/2, 20, Config::Colors::PALETTE_TEXT);
    EditorUI::nameInputBounds = {padding + labelWidth, currentY, fieldWidth, itemHeight};
    DrawRectangleRec(EditorUI::nameInputBounds, WHITE);
    DrawRectangleLinesEx(EditorUI::nameInputBounds, 1, activeTextField_ == 1 ? BLUE : DARKGRAY);
    DrawText(editorGateNameBuffer_, EditorUI::nameInputBounds.x + 5, EditorUI::nameInputBounds.y + (itemHeight - 20)/2 + 2, 20, BLACK);
    currentY += itemHeight + padding;

    DrawText("Category:", padding, currentY + (itemHeight - 20)/2, 20, Config::Colors::PALETTE_TEXT);
    EditorUI::categoryInputBounds = {padding + labelWidth, currentY, fieldWidth, itemHeight};
    DrawRectangleRec(EditorUI::categoryInputBounds, WHITE);
    DrawRectangleLinesEx(EditorUI::categoryInputBounds, 1, activeTextField_ == 2 ? BLUE : DARKGRAY);
    DrawText(editorCategoryBuffer_, EditorUI::categoryInputBounds.x + 5, EditorUI::categoryInputBounds.y + (itemHeight - 20)/2 + 2, 20, BLACK);
    currentY += itemHeight + padding;

    DrawText("Inputs:", padding, currentY + (itemHeight - 20)/2, 20, Config::Colors::PALETTE_TEXT);
    std::string numInputsStr = std::to_string(editorNumInputs_);
    float textWidthInputs = MeasureText(numInputsStr.c_str(), 20);
    float numDisplayX = EditorUI::nameInputBounds.x + buttonWidth + padding;
    float numDisplayWidth = fieldWidth - 2*buttonWidth - 2*padding;
    DrawText(numInputsStr.c_str(), numDisplayX + (numDisplayWidth - textWidthInputs)/2, currentY + (itemHeight - 20)/2 + 2, 20, BLACK);

    EditorUI::inputDecBounds = {EditorUI::nameInputBounds.x, currentY, buttonWidth, itemHeight};
    EditorUI::inputIncBounds = {EditorUI::nameInputBounds.x + fieldWidth - buttonWidth, currentY, buttonWidth, itemHeight};
    DrawRectangleRec(EditorUI::inputDecBounds, SKYBLUE); DrawText("-", EditorUI::inputDecBounds.x + (buttonWidth - MeasureText("-",20))/2, EditorUI::inputDecBounds.y + (itemHeight - 20)/2 + 2, 20, WHITE);
    DrawRectangleRec(EditorUI::inputIncBounds, SKYBLUE); DrawText("+", EditorUI::inputIncBounds.x + (buttonWidth - MeasureText("+",20))/2, EditorUI::inputIncBounds.y + (itemHeight - 20)/2 + 2, 20, WHITE);
    currentY += itemHeight + padding;

    DrawText("Outputs:", padding, currentY + (itemHeight - 20)/2, 20, Config::Colors::PALETTE_TEXT);
    std::string numOutputsStr = std::to_string(editorNumOutputs_);
    float textWidthOutputs = MeasureText(numOutputsStr.c_str(), 20);
    DrawText(numOutputsStr.c_str(), numDisplayX + (numDisplayWidth - textWidthOutputs)/2, currentY + (itemHeight - 20)/2 + 2, 20, BLACK);

    EditorUI::outputDecBounds = {EditorUI::nameInputBounds.x, currentY, buttonWidth, itemHeight};
    EditorUI::outputIncBounds = {EditorUI::nameInputBounds.x + fieldWidth - buttonWidth, currentY, buttonWidth, itemHeight};
    DrawRectangleRec(EditorUI::outputDecBounds, SKYBLUE); DrawText("-", EditorUI::outputDecBounds.x + (buttonWidth - MeasureText("-",20))/2, EditorUI::outputDecBounds.y + (itemHeight - 20)/2 + 2, 20, WHITE);
    DrawRectangleRec(EditorUI::outputIncBounds, SKYBLUE); DrawText("+", EditorUI::outputIncBounds.x + (buttonWidth - MeasureText("+",20))/2, EditorUI::outputIncBounds.y + (itemHeight - 20)/2 + 2, 20, WHITE);
    currentY += itemHeight + padding;

    currentY += padding;
    float singleButtonWidth = (fieldWidth - padding) / 2;
    EditorUI::saveButtonBounds = {EditorUI::nameInputBounds.x, currentY, singleButtonWidth, itemHeight + 10};
    EditorUI::cancelButtonBounds = {EditorUI::nameInputBounds.x + singleButtonWidth + padding, currentY, singleButtonWidth, itemHeight + 10};

    DrawRectangleRec(EditorUI::saveButtonBounds, LIME);
    DrawText("Save", EditorUI::saveButtonBounds.x + (EditorUI::saveButtonBounds.width - MeasureText("Save", 20))/2, EditorUI::saveButtonBounds.y + (EditorUI::saveButtonBounds.height - 20)/2 + 2, 20, DARKGREEN);
    DrawRectangleRec(EditorUI::cancelButtonBounds, PINK);
    DrawText("Cancel", EditorUI::cancelButtonBounds.x + (EditorUI::cancelButtonBounds.width - MeasureText("Cancel", 20))/2, EditorUI::cancelButtonBounds.y + (EditorUI::cancelButtonBounds.height - 20)/2 + 2, 20, MAROON);
    currentY += itemHeight + 10 + padding;

    // Status Message
    if (!editorStatusMessage_.empty()) {
        float messageWidth = MeasureText(editorStatusMessage_.c_str(), 18);
        Color messageColor = (editorStatusMessage_.rfind("Error:", 0) == 0) ? RED : DARKGREEN; // Check if message starts with "Error:"
        DrawText(editorStatusMessage_.c_str(),
                 EditorUI::nameInputBounds.x + (fieldWidth - messageWidth) / 2,
                 currentY, 18, messageColor);
        currentY += itemHeight;
    }

    DrawText("Pin Mappings", padding, currentY, 20, Config::Colors::PALETTE_TEXT);
    currentY += itemHeight;

    float mappingItemHeight = 25.0f;
    EditorUI::extInputPinBounds.assign(editorNumInputs_, {0,0,0,0});
    for (int i = 0; i < editorNumInputs_; ++i) {
        std::string label = "Ext. Input " + std::to_string(i) + ": ";
        EditorUI::extInputPinBounds[i] = {padding, currentY, sidebarWidth - 2*padding, mappingItemHeight};

        bool isCurrentlySelectedForMapping = isSelectingTargetForMapping_ &&
                                             mappingSelectedExtPinType_ == PinType::INPUT_PIN &&
                                             mappingSelectedExtPinIndex_ == i;
        if (isCurrentlySelectedForMapping) {
            DrawRectangleRec(EditorUI::extInputPinBounds[i], Fade(SKYBLUE, 0.7f));
        } else {
            DrawRectangleRec(EditorUI::extInputPinBounds[i], Fade(LIGHTGRAY, 0.3f));
        }
        DrawRectangleLinesEx(EditorUI::extInputPinBounds[i], 1, DARKGRAY);
        DrawText(label.c_str(), padding + 5, currentY + (mappingItemHeight - 18)/2, 18, BLACK);

        std::string mappingText = "None";
        for (const auto& mapping : currentCustomGateDefinition_.pinMappings) {
            if (!mapping.isOutputMapping && mapping.externalPinIndex == i) {
                mappingText = mapping.internalGateId + "[" + std::to_string(mapping.internalPinIndex) + "]";
                break;
            }
        }
        DrawText(mappingText.c_str(), padding + 5 + MeasureText(label.c_str(), 18), currentY + (mappingItemHeight - 18)/2, 18, DARKBLUE);
        currentY += mappingItemHeight + padding / 3;
    }

    currentY += padding;
    EditorUI::extOutputPinBounds.assign(editorNumOutputs_, {0,0,0,0});
    for (int i = 0; i < editorNumOutputs_; ++i) {
        std::string label = "Ext. Output " + std::to_string(i) + ": ";
        EditorUI::extOutputPinBounds[i] = {padding, currentY, sidebarWidth - 2*padding, mappingItemHeight};

        bool isCurrentlySelectedForMapping = isSelectingTargetForMapping_ &&
                                             mappingSelectedExtPinType_ == PinType::OUTPUT_PIN &&
                                             mappingSelectedExtPinIndex_ == i;
        if (isCurrentlySelectedForMapping) {
            DrawRectangleRec(EditorUI::extOutputPinBounds[i], Fade(SKYBLUE, 0.7f));
        } else {
            DrawRectangleRec(EditorUI::extOutputPinBounds[i], Fade(LIGHTGRAY, 0.3f));
        }
        DrawRectangleLinesEx(EditorUI::extOutputPinBounds[i], 1, DARKGRAY);
        DrawText(label.c_str(), padding + 5, currentY + (mappingItemHeight - 18)/2, 18, BLACK);

        std::string mappingText = "None";
        for (const auto& mapping : currentCustomGateDefinition_.pinMappings) {
            if (mapping.isOutputMapping && mapping.externalPinIndex == i) {
                mappingText = mapping.internalGateId + "[" + std::to_string(mapping.internalPinIndex) + "]";
                break;
            }
        }
        DrawText(mappingText.c_str(), padding + 5 + MeasureText(label.c_str(), 18), currentY + (mappingItemHeight - 18)/2, 18, DARKBLUE);
        currentY += mappingItemHeight + padding / 3;
    }

    if (customGateEditorPalette_) {
        customGateEditorPalette_->render(camera_);
    }
}


void UIManager::render() {
    BeginDrawing();
    ClearBackground(Config::Colors::BACKGROUND);

    if (currentUIMode_ == UIMode::CIRCUIT_VIEW) {
        updateCamera();

        BeginMode2D(camera_);

        if (Config::GRID_ENABLED) {
            renderGrid();
        }

        if (simulator_) {
            wireRenderer_->renderWires(simulator_->getWires());

            if (isDrawingWire_ && wireStartPin_) {
                Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), camera_);
                bool isOverInputPin = false;
                for (const auto& gate : simulator_->getGates()) {
                    if (!gate) continue;
                    for (size_t i = 0; i < gate->getInputPinCount(); i++) {
                        GatePin* pin = gate->getInputPin(i);
                        if (pin && pin->isMouseOverPin(worldMousePos) && !pin->isConnectedInput()) {
                            isOverInputPin = true;
                            break;
                        }
                    }
                    if (isOverInputPin) break;
                }
                wireRenderer_->renderWirePreview(
                    wireStartPin_->getAbsolutePosition(),
                    GetScreenToWorld2D(GetMousePosition(), camera_),
                    isOverInputPin,
                    Config::Colors::WIRE_PREVIEW,
                    Config::WIRE_THICKNESS_PREVIEW
                );
                gateRenderer_->renderWirePreview(
                    wireStartPin_,
                    simulator_->getGates(),
                    worldMousePos
                );
            }
            gateRenderer_->renderGates(simulator_->getGates(), camera_, selectedComponent_);
        }
        EndMode2D();

        if (paletteManager_) {
            paletteManager_->render(camera_);
        }

    } else if (currentUIMode_ == UIMode::CUSTOM_GATE_EDITOR) {
        renderCustomGateEditorInterface();

        BeginMode2D(camera_);
        BeginScissorMode(editorCanvasBounds_.x, editorCanvasBounds_.y, editorCanvasBounds_.width, editorCanvasBounds_.height);

        DrawRectangleRec(editorCanvasBounds_, Fade(DARKGRAY, 0.05f));

        if (customGateEditorSimulator_) {
            if (isSelectingTargetForMapping_) {
                for (const auto& gate : customGateEditorSimulator_->getGates()) {
                    if (!gate) continue;
                    if (mappingSelectedExtPinType_ == PinType::INPUT_PIN) {
                        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
                            GatePin* pin = gate->getInputPin(i);
                            if (pin) {
                                DrawCircleV(GetWorldToScreen2D(pin->getAbsolutePosition(), camera_), pin->getClickRadius() + 4, Fade(GREEN, 0.4f));
                            }
                        }
                    } else {
                         for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
                            GatePin* pin = gate->getOutputPin(i);
                            if (pin) {
                                DrawCircleV(GetWorldToScreen2D(pin->getAbsolutePosition(), camera_), pin->getClickRadius() + 4, Fade(ORANGE, 0.4f));
                            }
                        }
                    }
                }
            }

            gateRenderer_->renderGates(customGateEditorSimulator_->getGates(), camera_, selectedInternalGate_);
            wireRenderer_->renderWires(customGateEditorSimulator_->getWires());

            if (isDrawingWireInEditor_ && editorWireStartPin_) {
                Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), camera_);
                wireRenderer_->renderWirePreview(editorWireStartPin_->getAbsolutePosition(), worldMousePos, false, Config::Colors::WIRE_PREVIEW, Config::WIRE_THICKNESS_PREVIEW);
            }
        }
        EndScissorMode();
        EndMode2D();
    }

    DrawFPS(GetScreenWidth() - 90, 10);
    EndDrawing();
}

void UIManager::handleTextInput(char* buffer, int bufferSize, int charPressedVal) {
    if (charPressedVal > 0 && (charPressedVal >= 32 && charPressedVal <= 126) ) {
        int len = strlen(buffer);
        if (len < bufferSize - 1) {
            buffer[len] = (char)charPressedVal;
            buffer[len+1] = '\0';
        }
    }
}

void UIManager::deselectInternalGate() {
    if (selectedInternalGate_) {
        selectedInternalGate_ = nullptr;
    }
}

void UIManager::processCustomGateEditorInterfaceInput() {
    Vector2 mousePos = GetMousePosition();
    Vector2 editorWorldMousePos = GetScreenToWorld2D(mousePos, camera_);
    bool eventHandledThisFrame = false;

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (isDrawingWireInEditor_) {
            isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; eventHandledThisFrame = true;
        } else if (isSelectingTargetForMapping_) {
            isSelectingTargetForMapping_ = false; mappingSelectedExtPinIndex_ = -1; eventHandledThisFrame = true;
        } else {
            exitCustomGateEditorMode();
        }
        if (eventHandledThisFrame) { editorStatusMessage_ = "Action cancelled."; return; }
    }

    // Sidebar Click Handling (Properties, Pin Mappings, Save/Cancel)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(mousePos, editorCanvasBounds_)) {
        activeTextField_ = 0;
        editorStatusMessage_ = ""; // Clear status on new sidebar interaction

        for (int i = 0; i < editorNumInputs_; ++i) {
            if (CheckCollisionPointRec(mousePos, EditorUI::extInputPinBounds[i])) {
                isSelectingTargetForMapping_ = true; mappingSelectedExtPinIndex_ = i; mappingSelectedExtPinType_ = PinType::INPUT_PIN;
                isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; deselectInternalGate(); isDraggingInternalGate_ = false;
                eventHandledThisFrame = true; break;
            }
        }
        if (!eventHandledThisFrame) {
            for (int i = 0; i < editorNumOutputs_; ++i) {
                if (CheckCollisionPointRec(mousePos, EditorUI::extOutputPinBounds[i])) {
                    isSelectingTargetForMapping_ = true; mappingSelectedExtPinIndex_ = i; mappingSelectedExtPinType_ = PinType::OUTPUT_PIN;
                    isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; deselectInternalGate(); isDraggingInternalGate_ = false;
                    eventHandledThisFrame = true; break;
                }
            }
        }

        if (!eventHandledThisFrame) { // Only if an external pin wasn't clicked for mapping
            if (CheckCollisionPointRec(mousePos, EditorUI::nameInputBounds)) activeTextField_ = 1;
            else if (CheckCollisionPointRec(mousePos, EditorUI::categoryInputBounds)) activeTextField_ = 2;

            if (CheckCollisionPointRec(mousePos, EditorUI::inputIncBounds)) { editorNumInputs_ = std::min(16, editorNumInputs_ + 1); currentCustomGateDefinition_.numExternalInputPins = editorNumInputs_; EditorUI::extInputPinBounds.assign(editorNumInputs_, {0,0,0,0});}
            else if (CheckCollisionPointRec(mousePos, EditorUI::inputDecBounds)) { editorNumInputs_ = std::max(0, editorNumInputs_ - 1); currentCustomGateDefinition_.numExternalInputPins = editorNumInputs_; EditorUI::extInputPinBounds.assign(editorNumInputs_, {0,0,0,0});}
            else if (CheckCollisionPointRec(mousePos, EditorUI::outputIncBounds)) { editorNumOutputs_ = std::min(16, editorNumOutputs_ + 1); currentCustomGateDefinition_.numExternalOutputPins = editorNumOutputs_; EditorUI::extOutputPinBounds.assign(editorNumOutputs_, {0,0,0,0});}
            else if (CheckCollisionPointRec(mousePos, EditorUI::outputDecBounds)) { editorNumOutputs_ = std::max(0, editorNumOutputs_ - 1); currentCustomGateDefinition_.numExternalOutputPins = editorNumOutputs_; EditorUI::extOutputPinBounds.assign(editorNumOutputs_, {0,0,0,0});}
            else if (CheckCollisionPointRec(mousePos, EditorUI::saveButtonBounds)) {
                editorStatusMessage_ = ""; // Clear previous
                if (std::string(editorGateNameBuffer_).empty()) { editorStatusMessage_ = "Error: Gate name empty."; }
                else {
                    currentCustomGateDefinition_.name = editorGateNameBuffer_;
                    currentCustomGateDefinition_.category = editorCategoryBuffer_;
                    // Pin counts already updated above

                    bool allInputsMapped = true;
                    for (int i = 0; i < editorNumInputs_; ++i) {
                        bool mapped = false;
                        for(const auto& pm : currentCustomGateDefinition_.pinMappings) if (!pm.isOutputMapping && pm.externalPinIndex == i) { mapped = true; break; }
                        if (!mapped) { allInputsMapped = false; break; }
                    }
                    if (!allInputsMapped) { editorStatusMessage_ = "Error: All inputs must be mapped."; }
                    else {
                        bool allOutputsMapped = true;
                        for (int i = 0; i < editorNumOutputs_; ++i) {
                            bool mapped = false;
                            for(const auto& pm : currentCustomGateDefinition_.pinMappings) if (pm.isOutputMapping && pm.externalPinIndex == i) { mapped = true; break; }
                            if (!mapped) { allOutputsMapped = false; break; }
                        }
                        if (!allOutputsMapped) { editorStatusMessage_ = "Error: All outputs must be mapped."; }
                        else {
                            if (customGateRegistry_) {
                                if (customGateRegistry_->saveDefinition(currentCustomGateDefinition_)) {
                                    editorStatusMessage_ = "Gate saved!";
                                    if (paletteManager_) { // Refresh the main palette
                                        paletteManager_->refreshPaletteItems();
                                    }
                                    // No need to refresh customGateEditorPalette_ as we are exiting editor.
                                    // If we stayed in editor, and editor palette could show custom gates, then refresh it too.
                                    exitCustomGateEditorMode();
                                } else { editorStatusMessage_ = "Error: Save failed. Name conflict or I/O error."; }
                            } else { editorStatusMessage_ = "Error: Registry not available."; }
                        }
                    }
                }
            } else if (CheckCollisionPointRec(mousePos, EditorUI::cancelButtonBounds)) {
                exitCustomGateEditorMode();
            }
        }
        eventHandledThisFrame = true; // Any click in sidebar is handled here or by text input below
    }

    if (activeTextField_ != 0) {
        char* currentBuffer = (activeTextField_ == 1) ? editorGateNameBuffer_ : editorCategoryBuffer_;
        int bufferSize = (activeTextField_ == 1) ? sizeof(editorGateNameBuffer_) : sizeof(editorCategoryBuffer_);
        int charPressed = GetCharPressed();
        while(charPressed > 0) { handleTextInput(currentBuffer, bufferSize, charPressed); charPressed = GetCharPressed(); }
        if (IsKeyPressedRepeat(KEY_BACKSPACE) || IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(currentBuffer); if (len > 0) currentBuffer[len-1] = '\0';
        }
        if (activeTextField_ == 1) currentCustomGateDefinition_.name = editorGateNameBuffer_;
        else if (activeTextField_ == 2) currentCustomGateDefinition_.category = editorCategoryBuffer_;
    }

    if (eventHandledThisFrame && !isSelectingTargetForMapping_) {
         if (customGateEditorSimulator_) customGateEditorSimulator_->update();
         return;
    }

    bool interactionOnCanvas = CheckCollisionPointRec(mousePos, editorCanvasBounds_);
    if (!interactionOnCanvas && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !eventHandledThisFrame) {
        activeTextField_ = 0;
        if(isSelectingTargetForMapping_){ isSelectingTargetForMapping_ = false; mappingSelectedExtPinIndex_ = -1; }
    }

    if (interactionOnCanvas) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            activeTextField_ = 0;
            GatePin* clickedInternalPin = nullptr;

            if (customGateEditorSimulator_) {
                for (const auto& gate : customGateEditorSimulator_->getGates()) { /* ... (pin finding logic as before) ... */
                    if (!gate) continue;
                    if (isSelectingTargetForMapping_) { // Check pins relevant for mapping
                        PinType targetType = (mappingSelectedExtPinType_ == PinType::INPUT_PIN) ? PinType::INPUT_PIN : PinType::OUTPUT_PIN;
                        if (targetType == PinType::INPUT_PIN) {
                            for (size_t i = 0; i < gate->getInputPinCount(); ++i) if (gate->getInputPin(i) && gate->getInputPin(i)->isMouseOverPin(editorWorldMousePos)) { clickedInternalPin = gate->getInputPin(i); break;}
                        } else {
                            for (size_t i = 0; i < gate->getOutputPinCount(); ++i) if (gate->getOutputPin(i) && gate->getOutputPin(i)->isMouseOverPin(editorWorldMousePos)) { clickedInternalPin = gate->getOutputPin(i); break;}
                        }
                    } else if (!isDrawingWireInEditor_) { // Not mapping, not drawing: check output pins to start wire
                         for (size_t i = 0; i < gate->getOutputPinCount(); ++i) if (gate->getOutputPin(i) && gate->getOutputPin(i)->isMouseOverPin(editorWorldMousePos)) { clickedInternalPin = gate->getOutputPin(i); break;}
                    } else { // Drawing wire: check input pins to end wire
                         for (size_t i = 0; i < gate->getInputPinCount(); ++i) if (gate->getInputPin(i) && gate->getInputPin(i)->isMouseOverPin(editorWorldMousePos)) { clickedInternalPin = gate->getInputPin(i); break;}
                    }
                    if (clickedInternalPin) break;
                }
            }

            if (isSelectingTargetForMapping_) {
                if (clickedInternalPin) {
                    bool isValidTarget = (mappingSelectedExtPinType_ == PinType::INPUT_PIN && clickedInternalPin->getType() == PinType::INPUT_PIN) ||
                                         (mappingSelectedExtPinType_ == PinType::OUTPUT_PIN && clickedInternalPin->getType() == PinType::OUTPUT_PIN);
                    if (isValidTarget) {
                        currentCustomGateDefinition_.pinMappings.erase(
                            std::remove_if(currentCustomGateDefinition_.pinMappings.begin(), currentCustomGateDefinition_.pinMappings.end(),
                                [&](const CustomGateData::PinMappingDesc& m) {
                                    return m.externalPinIndex == mappingSelectedExtPinIndex_ &&
                                           ((mappingSelectedExtPinType_ == PinType::INPUT_PIN && !m.isOutputMapping) ||
                                            (mappingSelectedExtPinType_ == PinType::OUTPUT_PIN && m.isOutputMapping));
                                }),
                            currentCustomGateDefinition_.pinMappings.end());
                        CustomGateData::PinMappingDesc newMapping;
                        newMapping.externalPinIndex = mappingSelectedExtPinIndex_;
                        newMapping.internalGateId = clickedInternalPin->getParentGate()->getId();
                        newMapping.internalPinIndex = clickedInternalPin->getId();
                        newMapping.isOutputMapping = (mappingSelectedExtPinType_ == PinType::OUTPUT_PIN);
                        currentCustomGateDefinition_.pinMappings.push_back(newMapping);
                        editorStatusMessage_ = "Mapped ext. " + std::string(mappingSelectedExtPinType_ == PinType::INPUT_PIN ? "input" : "output") + " " + std::to_string(mappingSelectedExtPinIndex_) + " to " + newMapping.internalGateId + "[" + std::to_string(newMapping.internalPinIndex) + "]";
                        isSelectingTargetForMapping_ = false; mappingSelectedExtPinIndex_ = -1; eventHandledThisFrame = true;
                    } else { editorStatusMessage_ = "Error: Invalid pin type for mapping.";}
                } else { editorStatusMessage_ = "Select an internal pin to map."; }
            } else if (isDrawingWireInEditor_) {
                if (clickedInternalPin && editorWireStartPin_ && clickedInternalPin->getType() == PinType::INPUT_PIN && clickedInternalPin != editorWireStartPin_ && !clickedInternalPin->isConnectedInput()) {
                    Wire* newWire = customGateEditorSimulator_->createWire(editorWireStartPin_, clickedInternalPin);
                    if (newWire) {
                        CustomGateData::InternalWireDesc iwd;
                        iwd.fromGateId = editorWireStartPin_->getParentGate()->getId(); iwd.fromPinIndex = editorWireStartPin_->getId();
                        iwd.toGateId = clickedInternalPin->getParentGate()->getId(); iwd.toPinIndex = clickedInternalPin->getId();
                        currentCustomGateDefinition_.internalWires.push_back(iwd);
                    }
                    isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; eventHandledThisFrame = true;
                } else { isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; eventHandledThisFrame = true; }
            } else {
                if (clickedInternalPin && clickedInternalPin->getType() == PinType::OUTPUT_PIN) {
                    isDrawingWireInEditor_ = true; editorWireStartPin_ = clickedInternalPin;
                    isDraggingInternalGate_ = false; deselectInternalGate(); eventHandledThisFrame = true;
                } else if (!eventHandledThisFrame) {
                    deselectInternalGate();
                    if (customGateEditorSimulator_) {
                        const auto& gates = customGateEditorSimulator_->getGates();
                        for (auto it = gates.rbegin(); it != gates.rend(); ++it) {
                            LogicGate* gate = it->get();
                            if (gate && gate->isMouseOver(editorWorldMousePos)) {
                                selectedInternalGate_ = gate; isDraggingInternalGate_ = true;
                                internalGateDragStartOffset_ = Vector2Subtract(editorWorldMousePos, selectedInternalGate_->getPosition());
                                eventHandledThisFrame = true; break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (customGateEditorPalette_ && isDraggingFromEditorPalette_ && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !eventHandledThisFrame) {
        if (interactionOnCanvas) {
            Vector2 gatePosInEditorWorld = GetScreenToWorld2D(mousePos, camera_);
            LogicGate* newInternalGate = customGateEditorPalette_->endDraggingGate(gatePosInEditorWorld);
            if (newInternalGate) {
                CustomGateData::InternalGateDesc igd;
                igd.type = PaletteManager::getGateTypeName(customGateEditorPalette_->getDraggedGateType());
                igd.id = newInternalGate->getId();
                igd.posX = newInternalGate->getPosition().x; igd.posY = newInternalGate->getPosition().y;
                currentCustomGateDefinition_.internalGates.push_back(igd);
            }
        } else { customGateEditorPalette_->cancelDraggingGate(); }
        isDraggingFromEditorPalette_ = false; eventHandledThisFrame = true;
    }

    if (isDraggingInternalGate_ && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (selectedInternalGate_) {
            Vector2 newPos = Vector2Subtract(editorWorldMousePos, internalGateDragStartOffset_);
            selectedInternalGate_->setPosition(newPos);
            for (auto& desc : currentCustomGateDefinition_.internalGates) {
                if (desc.id == selectedInternalGate_->getId()) { desc.posX = newPos.x; desc.posY = newPos.y; break; }
            }
        }
    }
    if (isDraggingInternalGate_ && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDraggingInternalGate_ = false;

    if (isDrawingWireInEditor_ && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) { isDrawingWireInEditor_ = false; editorWireStartPin_ = nullptr; editorStatusMessage_ = "Wire cancelled.";}
    if (isSelectingTargetForMapping_ && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) { isSelectingTargetForMapping_ = false; mappingSelectedExtPinIndex_ = -1; editorStatusMessage_ = "Mapping cancelled.";}

    if (customGateEditorSimulator_) customGateEditorSimulator_->update();
}


void UIManager::processInput() {
    Vector2 mousePos = GetMousePosition();

    if (IsWindowResized()) {
        handleWindowResize(GetScreenWidth(), GetScreenHeight());
    }

    if (currentUIMode_ == UIMode::CUSTOM_GATE_EDITOR) {
        processCustomGateEditorInterfaceInput();
        return;
    }

    Vector2 worldMousePos = GetScreenToWorld2D(mousePos, camera_);
    if (paletteManager_ && paletteManager_->handleClick(mousePos)) {
        if (paletteManager_->wasCreateCustomGateActionTriggered()) {
            enterCustomGateEditorMode();
            return;
        }
        if (paletteManager_->getSelectedGateType() != GateType::NONE) {
            deselectAll();
        }
    } else if (paletteManager_ && paletteManager_->startDraggingGate(mousePos)) {
        // Dragging from palette
    } else {
        if (isDrawingWire_) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                GatePin* endPin = simulator_->findPinAt(worldMousePos);
                if (endPin && endPin != wireStartPin_ && endPin->getType() != wireStartPin_->getType() && !endPin->isConnectedInput()) {
                    completeWireDrawing(endPin);
                } else {
                    cancelWireDrawing();
                }
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsKeyPressed(KEY_ESCAPE)) {
                cancelWireDrawing();
            }
            updateWirePreview(worldMousePos);
        } else {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                GatePin* clickedPin = simulator_->findPinAt(worldMousePos);
                LogicGate* clickedGate = nullptr;
                Wire* clickedWire = nullptr;

                if (clickedPin) {
                    if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                        startDrawingWire(clickedPin);
                    } else {
                        selectComponent(clickedPin->getParentGate());
                    }
                } else {
                    clickedGate = simulator_->findGateAt(worldMousePos);
                    if (clickedGate) {
                        selectComponent(clickedGate);
                        startDraggingComponent(clickedGate, worldMousePos);

                        InputSource* inputSrc = dynamic_cast<InputSource*>(clickedGate);
                        if (inputSrc) {
                             clickedInputSource_ = inputSrc;
                             dragStartPosition_ = mousePos;
                        } else {
                            clickedInputSource_ = nullptr;
                        }
                    } else {
                        clickedWire = simulator_->findWireAt(worldMousePos);
                        if (clickedWire) {
                            selectWire(clickedWire);
                            if (startDraggingWirePoint(worldMousePos)) {
                            }
                        } else {
                            deselectAll();
                            startPanning(mousePos);
                        }
                    }
                }
            } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (isDraggingComponent_) stopDragging();
                if (isDraggingWirePoint_) stopWirePointDragging();
                if (isPanningActive()) stopPanning();
                if (clickedInputSource_ && !wasDragged(mousePos)) {
                    clickedInputSource_->toggleState();
                    simulator_->markGateDirty(clickedInputSource_);
                }
                clickedInputSource_ = nullptr;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if (isDraggingComponent_ || isDraggingWirePointActive()) {
                    updateDragging(worldMousePos);
                }
            }
        }
    }

    bool canStartPan = !isDrawingWire_ && !isDraggingComponent_ && !isDraggingWirePoint_ && !paletteManager_->isDraggingGateActive();
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON) || (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && canStartPan)) {
        if (!CheckCollisionPointRec(mousePos, paletteManager_->getPaletteBounds())) {
            startPanning(mousePos);
        }
    }

    if (isPanningActive()) {
        if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON) || IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            stopPanning();
        } else {
            updatePanning(mousePos);
        }
    }

    if (!CheckCollisionPointRec(mousePos, paletteManager_->getPaletteBounds())) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPosBeforeZoom = GetScreenToWorld2D(mousePos, camera_);
            camera_.zoom += wheel * Config::ZOOM_INCREMENT * camera_.zoom;
            if (camera_.zoom < Config::MIN_ZOOM) camera_.zoom = Config::MIN_ZOOM;
            if (camera_.zoom > Config::MAX_ZOOM) camera_.zoom = Config::MAX_ZOOM;
            Vector2 mouseWorldPosAfterZoom = GetScreenToWorld2D(mousePos, camera_);
            camera_.target = Vector2Add(camera_.target, Vector2Subtract(mouseWorldPosBeforeZoom, mouseWorldPosAfterZoom));
        }
    }

    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) deleteSelected();
    if (IsKeyPressed(KEY_SPACE)) {
        if (selectedComponent_ && dynamic_cast<InputSource*>(selectedComponent_)) {
            auto input = static_cast<InputSource*>(selectedComponent_);
            input->toggleState();
            simulator_->markGateDirty(input);
        }
    }
}


void UIManager::enterCustomGateEditorMode() {
    currentUIMode_ = UIMode::CUSTOM_GATE_EDITOR;
    currentCustomGateDefinition_ = CustomGateData{};

    strcpy(editorGateNameBuffer_, "MyCustomGate");
    strcpy(editorCategoryBuffer_, "Custom");
    editorNumInputs_ = 2;
    editorNumOutputs_ = 1;
    activeTextField_ = 0;

    currentCustomGateDefinition_.name = editorGateNameBuffer_;
    currentCustomGateDefinition_.category = editorCategoryBuffer_;
    currentCustomGateDefinition_.numExternalInputPins = editorNumInputs_;
    currentCustomGateDefinition_.numExternalOutputPins = editorNumOutputs_;

    customGateEditorSimulator_ = std::make_unique<CircuitSimulator>();
    customGateEditorPalette_ = std::make_unique<PaletteManager>(customGateEditorSimulator_, customGateRegistry_); // Pass registry
    customGateEditorPalette_->initialize();
    // TODO: Configure customGateEditorPalette_ to show only basic gates.

    float sidebarWidth = 250.0f;
    float topPanelHeight = 0.0f;
    float padding = 10.0f;
    editorCanvasBounds_ = {
        sidebarWidth + padding,
        topPanelHeight + padding,
        GetScreenWidth() - sidebarWidth - 2 * padding,
        GetScreenHeight() - topPanelHeight - 2 * padding
    };

    selectedInternalGate_ = nullptr;
    isDraggingInternalGate_ = false;
    isDraggingFromEditorPalette_ = false;
    isDrawingWireInEditor_ = false;
    editorWireStartPin_ = nullptr;
    mappingSelectedExtPinIndex_ = -1;
    isSelectingTargetForMapping_ = false;
    editorStatusMessage_ = "";

    deselectAll();
}

void UIManager::exitCustomGateEditorMode() {
    currentUIMode_ = UIMode::CIRCUIT_VIEW;
    customGateEditorSimulator_.reset();
    customGateEditorPalette_.reset();
    activeTextField_ = 0;
    selectedInternalGate_ = nullptr;
    isDraggingInternalGate_ = false;
    isDraggingFromEditorPalette_ = false;
    isDrawingWireInEditor_ = false;
    editorWireStartPin_ = nullptr;
    mappingSelectedExtPinIndex_ = -1;
    isSelectingTargetForMapping_ = false;
    editorStatusMessage_ = "";
}


void UIManager::deselectInternalGate() {
    if (selectedInternalGate_) {
        // selectedInternalGate_->setSelected(false);
        selectedInternalGate_ = nullptr;
    }
}


Camera2D& UIManager::getCamera() {
    return camera_;
}

Rectangle UIManager::getCanvasBounds() const {
    return canvasBounds_;
}

void UIManager::selectComponent(LogicGate* component) {
    deselectAll();

    if (component) {
        selectedComponent_ = component;
        selectedComponent_->setSelected(true);
    }
}

void UIManager::selectWire(Wire* wire) {
    deselectAll();

    if (wire) {
        selectedWire_ = wire;
        selectedWire_->setSelected(true);
    }
}

void UIManager::deselectAll() {
    if (selectedComponent_) {
        selectedComponent_->setSelected(false);
        selectedComponent_ = nullptr;
    }

    if (selectedWire_) {
        selectedWire_->setSelected(false);
        selectedWire_ = nullptr;
    }
}

void UIManager::startDrawingWire(GatePin* pin) {
    if (!pin || pin->getType() != PinType::OUTPUT_PIN) {
        return;
    }

    isDrawingWire_ = true;
    wireStartPin_ = pin;
    wirePreviewEndPos_ = pin->getAbsolutePosition();
    isDraggingComponent_ = false;
}

void UIManager::updateWirePreview(Vector2 mousePos) {
    if (!isDrawingWire_ || !wireStartPin_) {
        return;
    }

    wirePreviewEndPos_ = mousePos;
}

bool UIManager::completeWireDrawing(GatePin* pin) {
    if (!isDrawingWire_ || !wireStartPin_ || !pin) {
        return false;
    }

    if (pin == wireStartPin_ || pin->getType() != PinType::INPUT_PIN || pin->isConnectedInput()) {
        return false;
    }

    Wire* wire = simulator_->createWire(wireStartPin_, pin);

    isDrawingWire_ = false;
    wireStartPin_ = nullptr;

    return (wire != nullptr);
}

void UIManager::cancelWireDrawing() {
    isDrawingWire_ = false;
    wireStartPin_ = nullptr;
}

void UIManager::startDraggingComponent(LogicGate* component, Vector2 mousePos) {
    if (!component) {
        return;
    }

    selectComponent(component);
    isDraggingComponent_ = true;
    dragStartOffset_ = Vector2Subtract(mousePos, component->getPosition());
    dragStartPosition_ = mousePos;
}

void UIManager::updateDragging(Vector2 mousePos) {
    if (!isDraggingComponent_ || !selectedComponent_) {
        return;
    }

    Vector2 position = Vector2Subtract(mousePos, dragStartOffset_);
    Vector2 alignedPosition = checkWireAlignmentSnapping(selectedComponent_, position);
    selectedComponent_->setPosition(alignedPosition);
    updateWirePathsForComponent(selectedComponent_);
}

void UIManager::stopDragging() {
    if (isDraggingComponent_ && selectedComponent_) {
        Vector2 currentPos = selectedComponent_->getPosition();
        Vector2 alignedPosition = checkWireAlignmentSnapping(selectedComponent_, currentPos);
        selectedComponent_->setPosition(alignedPosition);
        updateWirePathsForComponent(selectedComponent_);
    }

    isDraggingComponent_ = false;
}

bool UIManager::startDraggingWirePoint(Vector2 mousePos) {
    if (!selectedWire_) {
        return false;
    }

    if (selectedWire_->startDraggingPoint(mousePos)) {
        isDraggingWirePoint_ = true;
        return true;
    }

    return false;
}

void UIManager::updateWirePointDragging(Vector2 mousePos) {
    if (!isDraggingWirePoint_ || !selectedWire_) {
        return;
    }

    selectedWire_->updateDraggedPoint(mousePos);
}

void UIManager::stopWirePointDragging() {
    if (selectedWire_) {
        selectedWire_->stopDraggingPoint();
    }
    isDraggingWirePoint_ = false;
}

bool UIManager::isDraggingWirePointActive() const {
    return isDraggingWirePoint_;
}

void UIManager::deleteSelected() {
    if (selectedComponent_) {
        simulator_->removeGate(selectedComponent_);
        selectedComponent_ = nullptr;
    } else if (selectedWire_) {
        simulator_->removeWire(selectedWire_);
        selectedWire_ = nullptr;
    }
}

LogicGate* UIManager::getSelectedComponent() const {
    return selectedComponent_;
}

Wire* UIManager::getSelectedWire() const {
    return selectedWire_;
}

bool UIManager::isDrawingWireActive() const {
    return isDrawingWire_;
}

PaletteManager& UIManager::getPaletteManager() {
    return *paletteManager_;
}

void UIManager::setClickedInputSource(InputSource* inputSource) {
    clickedInputSource_ = inputSource;
}

bool UIManager::wasDragged(Vector2 currentMousePos) const {
    float dx = currentMousePos.x - dragStartPosition_.x;
    float dy = currentMousePos.y - dragStartPosition_.y;
    float distance = sqrt(dx*dx + dy*dy);
    return distance > Config::DRAG_THRESHOLD;
}

void UIManager::renderGrid() {
    Vector2 screenTopLeft = GetScreenToWorld2D({0, 0}, camera_);
    Vector2 screenBottomRight = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera_);

    int startX = floor(screenTopLeft.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int startY = floor(screenTopLeft.y / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endX = ceil(screenBottomRight.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endY = ceil(screenBottomRight.y / Config::GRID_SIZE) * Config::GRID_SIZE;

    float majorGridSize = Config::GRID_SIZE * 4.0f;
    int majorStartX = floor(screenTopLeft.x / majorGridSize) * majorGridSize;
    int majorStartY = floor(screenTopLeft.y / majorGridSize) * majorGridSize;
    int majorEndX = ceil(screenBottomRight.x / majorGridSize) * majorGridSize;
    int majorEndY = ceil(screenBottomRight.y / majorGridSize) * majorGridSize;

    for (float x = majorStartX; x <= majorEndX; x += majorGridSize) {
        DrawLineV({x, screenTopLeft.y}, {x, screenBottomRight.y}, Config::Colors::GRID_LINE);
    }
    for (float y = majorStartY; y <= majorEndY; y += majorGridSize) {
        DrawLineV({screenTopLeft.x, y}, {screenBottomRight.x, y}, Config::Colors::GRID_LINE);
    }

    float dotSize = 1.5f / camera_.zoom;
    dotSize = std::max(0.5f, std::min(2.0f, dotSize));

    for (float x = startX; x <= endX; x += Config::GRID_SIZE) {
        for (float y = startY; y <= endY; y += Config::GRID_SIZE) {
            bool isMajorIntersection = (fmod(x, majorGridSize) == 0.0f && fmod(y, majorGridSize) == 0.0f);
            Color dotColor = isMajorIntersection ? Config::Colors::GRID_LINE : Config::Colors::GRID_DOT;
            float currentDotSize = isMajorIntersection ? dotSize * 1.5f : dotSize;

            DrawCircleV({x, y}, currentDotSize, dotColor);
        }
    }
}

void UIManager::startPanning(Vector2 mousePos) {
    if (!isPanning_) {
        isPanning_ = true;
        panStartPosition_ = mousePos;
        lastMousePosition_ = mousePos;
        panVelocity_ = {0, 0};
    }
}

void UIManager::updatePanning(Vector2 mousePos) {
    if (isPanning_) {
        Vector2 delta = {
            mousePos.x - lastMousePosition_.x,
            mousePos.y - lastMousePosition_.y
        };

        camera_.target.x -= delta.x / camera_.zoom;
        camera_.target.y -= delta.y / camera_.zoom;

        panVelocity_.x = delta.x / camera_.zoom;
        panVelocity_.y = delta.y / camera_.zoom;

        lastMousePosition_ = mousePos;
    }
}

void UIManager::stopPanning() {
    isPanning_ = false;
}

void UIManager::updateCamera() {
    if (!isPanning_ && (panVelocity_.x != 0 || panVelocity_.y != 0)) {
        camera_.target.x -= panVelocity_.x;
        camera_.target.y -= panVelocity_.y;

        panVelocity_.x *= panInertia_;
        panVelocity_.y *= panInertia_;

        if (fabs(panVelocity_.x) < 0.01f) panVelocity_.x = 0;
        if (fabs(panVelocity_.y) < 0.01f) panVelocity_.y = 0;
    }
}

bool UIManager::isPanningActive() const {
    return isPanning_;
}

void UIManager::updateWirePathsForComponent(LogicGate* component) {
    if (!component) {
        return;
    }

    const std::vector<Wire*>& associatedWires = component->getAssociatedWires();

    for (Wire* wire : associatedWires) {
        if (wire) {
            wire->recalculatePath();
        }
    }
}


void UIManager::handleWindowResize(int newWidth, int newHeight) {
    camera_.offset = { (float)newWidth / 2, (float)newHeight / 2 };

    canvasBounds_ = {
        Config::PALETTE_WIDTH,
        0,
        (float)newWidth - Config::PALETTE_WIDTH,
        (float)newHeight
    };

    // Update editor canvas bounds too
    float sidebarWidth = 250.0f;
    float topPanelHeight = 0.0f;
    float padding = 10.0f;
    editorCanvasBounds_ = {
        sidebarWidth + padding,
        topPanelHeight + padding,
        (float)newWidth - sidebarWidth - 2 * padding,
        (float)newHeight - topPanelHeight - 2 * padding
    };


    paletteManager_->handleWindowResize();
    if (customGateEditorPalette_) { // If editor palette exists
        customGateEditorPalette_->handleWindowResize(); // Assuming it has this method
    }
}

Vector2 UIManager::checkWireAlignmentSnapping(LogicGate* gate, Vector2 position) {
    if (!gate) {
        return position;
    }

    const float SNAP_THRESHOLD = 15.0f;
    const std::vector<Wire*>& associatedWires = gate->getAssociatedWires();

    if (associatedWires.empty()) {
        return position;
    }

    Vector2 adjustedPosition = position;
    bool hasSnapped = false;

    for (Wire* wire : associatedWires) {
        if (!wire || hasSnapped) continue;

        GatePin* sourcePin = wire->getSourcePin();
        GatePin* destPin = wire->getDestPin();

        if (!sourcePin || !destPin) continue;

        LogicGate* sourceGate = sourcePin->getParentGate();
        LogicGate* destGate = destPin->getParentGate();

        if (!sourceGate || !destGate) continue;

        GatePin* thisGatePin;
        GatePin* otherGatePin;

        if (gate == sourceGate) {
            thisGatePin = sourcePin;
            otherGatePin = destPin;
        } else {
            thisGatePin = destPin;
            otherGatePin = sourcePin;
        }

        Vector2 currentPinOffset = Vector2Subtract(thisGatePin->getAbsolutePosition(), gate->getPosition());
        Vector2 projectedPinPos = Vector2Add(position, currentPinOffset);
        Vector2 otherPinPos = otherGatePin->getAbsolutePosition();

        float yDiff = fabs(projectedPinPos.y - otherPinPos.y);
        if (yDiff < SNAP_THRESHOLD) {
            float yAdjustment = otherPinPos.y - projectedPinPos.y;
            adjustedPosition.y = position.y + yAdjustment;
            hasSnapped = true;
        }

        if (!hasSnapped) {
            float xDiff = fabs(projectedPinPos.x - otherPinPos.x);
            if (xDiff < SNAP_THRESHOLD) {
                float xAdjustment = otherPinPos.x - projectedPinPos.x;
                adjustedPosition.x = position.x + xAdjustment;
                hasSnapped = true;
            }
        }
    }

    return adjustedPosition;
}
