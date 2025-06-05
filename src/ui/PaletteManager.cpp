#include "ui/PaletteManager.h"
#include "ui/VisualEffects.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "core/DerivedGates.h"
#include "core/CustomGate.h"         // Added for creating CustomGate instances
#include "core/CustomGateRegistry.h" // Added for accessing definitions
#include <utility>
#include <iostream> // For debugging

PaletteManager::PaletteManager(std::shared_ptr<CircuitSimulator> sim, CustomGateRegistry* registry)
    : selectedGateType_(GateType::NONE), simulator_(sim), customGateRegistry_(registry), // Store registry
      isDraggingGate_(false), draggedGateType_(GateType::NONE),
      dragStartPos_({0, 0}), currentDragPos_({0, 0}),
      createCustomGateActionTriggered_(false) {
    draggedCustomGateName_.clear(); // Initialize
}

void PaletteManager::initialize() {
    // The actual population of items is now done in refreshPaletteItems
    refreshPaletteItems();
}

void PaletteManager::refreshPaletteItems() {
    gatePalette_.clear();
    float currentY = Config::CANVAS_PADDING;
    // Add space for the title if there is one (consistent with UIManager's editor palette init)
    currentY += 50; // Approximate height of a title area, adjust if PaletteManager has own title

    GateType standardTypes[] = {
        GateType::INPUT_SOURCE, GateType::OUTPUT_SINK,
        GateType::AND, GateType::OR, GateType::XOR, GateType::NOT,
        GateType::CREATE_CUSTOM_GATE
    };

    for (GateType type : standardTypes) {
        gatePalette_.push_back({
            {
                Config::CANVAS_PADDING,
                currentY,
                Config::PALETTE_WIDTH - 2 * Config::CANVAS_PADDING,
                Config::PALETTE_ITEM_HEIGHT
            },
            type,
            getGateTypeName(type), // Label comes from getGateTypeName
            "" // customGateName is empty for standard types
        });
        currentY += Config::PALETTE_ITEM_HEIGHT + Config::CANVAS_PADDING;
    }

    // Add custom gates from the registry
    if (customGateRegistry_) {
        const auto& allDefinitions = customGateRegistry_->getAllDefinitions();
        if (!allDefinitions.empty()) {
             // Optional: Add a small separator or title for custom gates
            currentY += Config::CANVAS_PADDING; // Extra padding
            // Could draw a text label here in render() if needed: "Custom Gates"
        }

        for (const auto& pair : allDefinitions) {
            const CustomGateData& def = pair.second;
            PaletteItem item;
            item.type = GateType::CUSTOM_GATE_INSTANCE;
            item.label = def.name; // Use definition name as label
            item.customGateName = def.name; // Store the key for lookup
            item.bounds = {
                Config::CANVAS_PADDING,
                currentY,
                Config::PALETTE_WIDTH - 2 * Config::CANVAS_PADDING,
                Config::PALETTE_ITEM_HEIGHT
            };
            gatePalette_.push_back(item);
            currentY += Config::PALETTE_ITEM_HEIGHT + Config::CANVAS_PADDING;
        }
    }
}


void PaletteManager::render(const Camera2D& camera) {
    // Draw enhanced palette background with gradient
    VisualEffects::drawRoundedRectangleGradient(
        {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()},
        0.0f, Config::Colors::PALETTE, Config::Colors::SURFACE
    );

    // Draw subtle border
    DrawLineEx({Config::PALETTE_WIDTH, 0}, {Config::PALETTE_WIDTH, (float)GetScreenHeight()},
               2.0f, Config::Colors::GRID_LINE);

    // Draw enhanced palette title with shadow
    Rectangle titleBounds = {0, 0, Config::PALETTE_WIDTH, 50};
    VisualEffects::drawRoundedRectangleGradient(titleBounds, 0.0f,
                                              Config::Colors::SURFACE_VARIANT, Config::Colors::SURFACE);

    VisualEffects::drawTextWithShadow("COMPONENTS", {15, 15}, 22,
                                    Config::Colors::PALETTE_TEXT, Config::Colors::GATE_SHADOW,
                                    {1.0f, 1.0f});

    Vector2 mousePos = GetMousePosition();

    for (const auto& item : gatePalette_) {
        bool isHovered = CheckCollisionPointRec(mousePos, item.bounds);
        bool isSelected = (selectedGateType_ == item.type && (item.type != GateType::CUSTOM_GATE_INSTANCE || draggedCustomGateName_ == item.customGateName) );

        // For CREATE_CUSTOM_GATE, it's never "selected" in a way that it can be dragged.
        // Its click is an action.
        if (item.type == GateType::CREATE_CUSTOM_GATE) isSelected = false;


        Color bgColor;
        if (isSelected && item.type != GateType::CREATE_CUSTOM_GATE) { // Don't show CREATE_CUSTOM_GATE as selected for dragging
            bgColor = Config::Colors::PALETTE_ITEM_SELECTED;
        } else if (isHovered) {
            bgColor = Config::Colors::PALETTE_ITEM_HOVER;
        } else {
            bgColor = Config::Colors::PALETTE_ITEM;
        }
        if (item.type == GateType::CREATE_CUSTOM_GATE && isHovered) { // Special hover for button
             bgColor = Config::Colors::BUTTON_HOVER;
        }


        Vector2 shadowOffset = {2.0f, 2.0f};
        VisualEffects::drawRoundedRectangleWithShadow(item.bounds, 0.3f, bgColor,
                                                    Config::Colors::GATE_SHADOW, shadowOffset);

        if (isSelected && item.type != GateType::CREATE_CUSTOM_GATE) {
            Color glowColor = Fade(Config::Colors::SELECTION_HIGHLIGHT, 0.3f);
            Rectangle expandedBounds = { item.bounds.x - 2.0f, item.bounds.y - 2.0f, item.bounds.width + 4.0f, item.bounds.height + 4.0f };
            DrawRectangleRounded(expandedBounds, 0.3f, 8, glowColor);
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::SELECTION_HIGHLIGHT);
        } else if (isHovered) {
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::PRIMARY_VARIANT);
        } else {
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::GATE_OUTLINE);
        }

        Rectangle iconBounds = { item.bounds.x + 10, item.bounds.y + (item.bounds.height - 35) / 2, 35, 35 };
        Color iconColor = getGateColor(item.type);
        drawGateIcon(item.type, iconBounds, iconColor);

        Vector2 textSize = MeasureTextEx(GetFontDefault(), item.label.c_str(), 22, 1);
        DrawText( item.label.c_str(), item.bounds.x + 55 + (item.bounds.width - 55 - textSize.x) / 2,
                  item.bounds.y + (item.bounds.height - textSize.y) / 2, 22, Config::Colors::PALETTE_TEXT );
    }

    DrawRectangle(0, GetScreenHeight() - 80, Config::PALETTE_WIDTH, 80, Fade(BLACK, 0.5f));
    DrawText("Drag: Place", 10, GetScreenHeight() - 70, 18, LIGHTGRAY);
    DrawText("Middle/Right: Pan", 10, GetScreenHeight() - 50, 18, LIGHTGRAY);
    DrawText("Backspace: Remove", 10, GetScreenHeight() - 30, 18, LIGHTGRAY);

    if (isDraggingGate_ && draggedGateType_ != GateType::NONE && draggedGateType_ != GateType::CREATE_CUSTOM_GATE) {
        Vector2 worldPos = GetScreenToWorld2D(currentDragPos_, camera);
        Vector2 screenPos = GetWorldToScreen2D(worldPos, camera); // Use main camera for preview
        Rectangle previewBounds = { screenPos.x - Config::DRAG_PREVIEW_SIZE / 2, screenPos.y - Config::DRAG_PREVIEW_SIZE / 2, Config::DRAG_PREVIEW_SIZE, Config::DRAG_PREVIEW_SIZE };
        Color previewColor = getGateColor(draggedGateType_);
        previewColor.a = static_cast<unsigned char>(255 * Config::DRAG_PREVIEW_ALPHA);
        drawGateIcon(draggedGateType_, previewBounds, previewColor);
        DrawText("+", screenPos.x - 5, screenPos.y - 10, 22, WHITE);
    }
}

bool PaletteManager::handleClick(Vector2 mousePos) {
    Rectangle paletteBounds = {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()};
    if (!CheckCollisionPointRec(mousePos, paletteBounds)) {
        return false;
    }

    for (const auto& item : gatePalette_) {
        if (CheckCollisionPointRec(mousePos, item.bounds)) {
            if (item.type == GateType::CREATE_CUSTOM_GATE) {
                createCustomGateActionTriggered_ = true;
                selectedGateType_ = GateType::NONE;
                draggedCustomGateName_.clear();
            } else {
                selectedGateType_ = item.type;
                if (item.type == GateType::CUSTOM_GATE_INSTANCE) {
                    draggedCustomGateName_ = item.customGateName; // Store name for custom gate selection
                } else {
                    draggedCustomGateName_.clear();
                }
            }
            return true;
        }
    }
    return false;
}

std::unique_ptr<LogicGate> PaletteManager::createSelectedGateInstance(Vector2 position) {
    // For CUSTOM_GATE_INSTANCE, createGateInstance now uses draggedCustomGateName_
    return createGateInstance(selectedGateType_, position);
}

GateType PaletteManager::getSelectedGateType() const {
    return selectedGateType_;
}

void PaletteManager::setSelectedGateType(GateType type) {
    selectedGateType_ = type;
    if (type != GateType::CUSTOM_GATE_INSTANCE) { // Clear custom name if not a custom instance type
        draggedCustomGateName_.clear();
    }
    // If type IS CUSTOM_GATE_INSTANCE, draggedCustomGateName_ should have been set by handleClick or startDraggingGate
}

std::string PaletteManager::getGateTypeName(GateType type) {
    switch (type) {
        case GateType::INPUT_SOURCE: return "Input";
        case GateType::OUTPUT_SINK:  return "Output";
        case GateType::AND:          return "AND";
        case GateType::OR:           return "OR";
        case GateType::XOR:          return "XOR";
        case GateType::NOT:          return "NOT";
        case GateType::CREATE_CUSTOM_GATE: return "New Custom...";
        case GateType::CUSTOM_GATE_INSTANCE: return "Custom Gate"; // Generic name, specific name is in item.label
        default:                     return "Unknown";
    }
}

Rectangle PaletteManager::getPaletteBounds() const {
    return {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()};
}

void PaletteManager::handleWindowResize() {
    // Palette items are typically fixed relative to top-left, so may not need updates
    // unless their width/height is relative to screen size, which is not current setup.
    // If PaletteManager itself has a variable width based on screen, then item bounds need recalculation.
    // For now, assuming fixed width palette.
    // refreshPaletteItems(); // Could be called if item layout depends on screen height for vertical scrolling etc.
}

bool PaletteManager::startDraggingGate(Vector2 mousePos) {
    Rectangle paletteBounds = {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()};
    if (!CheckCollisionPointRec(mousePos, paletteBounds)) {
        return false;
    }

    for (const auto& item : gatePalette_) {
        if (CheckCollisionPointRec(mousePos, item.bounds)) {
            if (item.type == GateType::CREATE_CUSTOM_GATE) {
                return false; // This button is an action, not draggable
            }
            isDraggingGate_ = true;
            draggedGateType_ = item.type;
            if (item.type == GateType::CUSTOM_GATE_INSTANCE) {
                draggedCustomGateName_ = item.customGateName;
            } else {
                draggedCustomGateName_.clear();
            }
            dragStartPos_ = mousePos;
            currentDragPos_ = mousePos;
            // Also update selectedGateType_ to reflect what's being dragged
            selectedGateType_ = item.type;
            return true;
        }
    }
    return false;
}

void PaletteManager::updateDragPosition(Vector2 mousePos) {
    if (isDraggingGate_) {
        currentDragPos_ = mousePos;
    }
}

LogicGate* PaletteManager::endDraggingGate(Vector2 worldPos) {
    if (!isDraggingGate_ || draggedGateType_ == GateType::NONE || draggedGateType_ == GateType::CREATE_CUSTOM_GATE) {
        isDraggingGate_ = false; // Ensure reset even if no gate created
        draggedGateType_ = GateType::NONE;
        draggedCustomGateName_.clear();
        return nullptr;
    }

    auto newGate = createGateInstance(draggedGateType_, worldPos); // Uses draggedGateType_ and potentially draggedCustomGateName_
    LogicGate* rawPtr = nullptr;

    if (newGate) {
        rawPtr = simulator_->addGate(std::move(newGate)); // Add to the simulator this palette is associated with
    }

    isDraggingGate_ = false;
    draggedGateType_ = GateType::NONE;
    // Don't clear selectedGateType_ here, it might be useful for placing multiple gates.
    // Clear draggedCustomGateName_ as the drag operation is over.
    draggedCustomGateName_.clear();

    return rawPtr;
}

void PaletteManager::cancelDraggingGate() {
    isDraggingGate_ = false;
    draggedGateType_ = GateType::NONE;
    draggedCustomGateName_.clear();
}

bool PaletteManager::isDraggingGateActive() const {
    return isDraggingGate_;
}

GateType PaletteManager::getDraggedGateType() const {
    return draggedGateType_;
}

Vector2 PaletteManager::getCurrentDragPosition() const {
    return currentDragPos_;
}

std::unique_ptr<LogicGate> PaletteManager::createGateInstance(GateType type, Vector2 position) {
    if (type == GateType::NONE || type == GateType::CREATE_CUSTOM_GATE) {
        return nullptr;
    }

    std::string idPrefix = "gate";
    if (type == GateType::INPUT_SOURCE) idPrefix = "input";
    else if (type == GateType::OUTPUT_SINK) idPrefix = "output";
    else if (type == GateType::CUSTOM_GATE_INSTANCE) idPrefix = "cg";


    switch (type) {
        case GateType::INPUT_SOURCE:
            return std::make_unique<InputSource>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Vector2{Config::INPUT_OUTPUT_SIZE, Config::INPUT_OUTPUT_SIZE}, "IN" );
        case GateType::OUTPUT_SINK:
            return std::make_unique<OutputSink>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Config::INPUT_OUTPUT_SIZE / 2, "OUT" );
        case GateType::AND:
            return std::make_unique<AndGate>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT );
        case GateType::OR:
            return std::make_unique<OrGate>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT );
        case GateType::XOR:
            return std::make_unique<XorGate>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT );
        case GateType::NOT:
            return std::make_unique<NotGate>( idPrefix + std::to_string(simulator_->useNextGateId()), position,
                Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT );
        case GateType::CUSTOM_GATE_INSTANCE:
            if (customGateRegistry_ && !draggedCustomGateName_.empty()) {
                CustomGateData definition;
                if (customGateRegistry_->getDefinition(draggedCustomGateName_, definition)) {
                    // Sanitize name for ID is good practice, but for now direct use.
                    std::string idStr = idPrefix + "_" + draggedCustomGateName_ + "_" + std::to_string(simulator_->useNextGateId());
                    return std::make_unique<CustomGate>(idStr, position, definition);
                } else {
                    std::cerr << "Error: PaletteManager - Failed to get definition for custom gate: " << draggedCustomGateName_ << std::endl;
                }
            }
            return nullptr;
        default:
            return nullptr;
    }
}

Color PaletteManager::getGateColor(GateType type) const {
    switch (type) {
        case GateType::INPUT_SOURCE: return Config::Colors::INPUT_ON;
        case GateType::OUTPUT_SINK:  return Config::Colors::OUTPUT_ON;
        case GateType::AND:          return Config::Colors::AND_GATE;
        case GateType::OR:           return Config::Colors::OR_GATE;
        case GateType::XOR:          return Config::Colors::XOR_GATE;
        case GateType::NOT:          return Config::Colors::NOT_GATE;
        case GateType::CREATE_CUSTOM_GATE: return Config::Colors::BUTTON_DEFAULT;
        case GateType::CUSTOM_GATE_INSTANCE: return Config::Colors::CUSTOM_GATE; // Assuming this color is defined
        default:                     return Config::Colors::GATE_FILL;
    }
}

void PaletteManager::drawGateIcon(GateType type, Rectangle bounds, Color color) const {
    switch (type) {
        case GateType::CREATE_CUSTOM_GATE:
            DrawText("+", bounds.x + bounds.width / 2 - MeasureText("+", 20) / 2, bounds.y + bounds.height / 2 - 10, 20, BLACK);
            break;
        case GateType::CUSTOM_GATE_INSTANCE:
            DrawRectangleRounded(bounds, 0.2f, 4, color); // Simple box for custom gate
            DrawText("CG", bounds.x + bounds.width/2 - MeasureText("CG", (int)(bounds.height*0.5f))/2, bounds.y + bounds.height/2 - (int)(bounds.height*0.5f)/2, (int)(bounds.height*0.5f), BLACK);
            DrawRectangleRoundedLines(bounds, 0.2f, 4, BLACK);
            break;
        // ... other cases from original file ...
        case GateType::INPUT_SOURCE:
            DrawRectangleRounded(bounds, 0.3f, 4, color);
            DrawRectangleRoundedLines(bounds, 0.3f, 4, BLACK);
            break;
        case GateType::OUTPUT_SINK:
            { Vector2 center = {bounds.x + bounds.width/2, bounds.y + bounds.height/2}; float radius = bounds.width/2 - 2;
                DrawCircleV(center, radius, color); DrawCircleLines(center.x, center.y, radius, BLACK); }
            break;
        case GateType::AND: {
                float radius = bounds.height / 2.0f; float flatPartWidth = bounds.width * 0.6f;
                Vector2 center = {bounds.x + flatPartWidth, bounds.y + bounds.height/2};
                Rectangle rectPart = { bounds.x, bounds.y, flatPartWidth, bounds.height }; DrawRectangleRec(rectPart, color);
                int segments = 20;
                for (int i = 0; i < segments; i++) {
                    float startAngle = PI/2 - (i * PI / segments); float endAngle = PI/2 - ((i + 1) * PI / segments);
                    DrawTriangle(center, { center.x + radius * cosf(startAngle), center.y + radius * sinf(startAngle) }, { center.x + radius * cosf(endAngle), center.y + radius * sinf(endAngle) }, color);
                }
                DrawLineEx({bounds.x, bounds.y}, {bounds.x, bounds.y + bounds.height}, 1.0f, BLACK);
                DrawLineEx({bounds.x, bounds.y}, {center.x, bounds.y}, 1.0f, BLACK);
                DrawLineEx({bounds.x, bounds.y + bounds.height}, {center.x, bounds.y + bounds.height}, 1.0f, BLACK);
                for (int i = 0; i < segments; i++) {
                    float startAngle = PI/2 - (i * PI / segments); float endAngle = PI/2 - ((i + 1) * PI / segments);
                    DrawLineEx({ center.x + radius * cosf(startAngle), center.y + radius * sinf(startAngle) }, { center.x + radius * cosf(endAngle), center.y + radius * sinf(endAngle) }, 1.0f, BLACK);
                }
            } break;
        case GateType::OR: {
                float actualWidth = bounds.width * 0.85f; float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f; float rightX = leftX + actualWidth;
                Vector2 rightPoint = {rightX, bounds.y + bounds.height/2};
                const int segments = 10; float curveDepth = bounds.height * 0.2f;
                Vector2 curvePoints[segments + 1];
                for (int i = 0; i <= segments; i++) {
                    float t = (float)i / segments; float y = bounds.y + t * bounds.height;
                    float normalizedT = 2.0f * t - 1.0f; float curveAmount = 1.0f - normalizedT * normalizedT;
                    float x = leftX + curveDepth * curveAmount; curvePoints[i] = {x, y};
                }
                for (int i = 0; i < segments; i++) DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, color);
                for (int i = 0; i < segments; i++) DrawLineEx(curvePoints[i], curvePoints[i+1], 1.0f, BLACK);
                DrawLineEx(curvePoints[0], rightPoint, 1.0f, BLACK); DrawLineEx(curvePoints[segments], rightPoint, 1.0f, BLACK);
            } break;
        case GateType::XOR: {
                float actualWidth = bounds.width * 0.8f; float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f; float rightX = leftX + actualWidth;
                Vector2 rightPoint = {rightX, bounds.y + bounds.height/2};
                const int segments = 10; float curveDepth = bounds.height * 0.2f; float curveOffset = bounds.height * 0.2f;
                Vector2 curvePoints[segments + 1];
                for (int i = 0; i <= segments; i++) {
                    float t = (float)i / segments; float y = bounds.y + t * bounds.height;
                    float normalizedT = 2.0f * t - 1.0f; float curveAmount = 1.0f - normalizedT * normalizedT;
                    float x = leftX + curveDepth * curveAmount; curvePoints[i] = {x, y};
                }
                for (int i = 0; i < segments; i++) DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, color);
                for (int i = 0; i < segments; i++) DrawLineEx(curvePoints[i], curvePoints[i+1], 1.0f, BLACK);
                DrawLineEx(curvePoints[0], rightPoint, 1.0f, BLACK); DrawLineEx(curvePoints[segments], rightPoint, 1.0f, BLACK);
                Vector2 prevPoint = {leftX - curveOffset, bounds.y};
                for (int i = 1; i <= segments; i++) {
                    float t = (float)i / segments; float y = bounds.y + t * bounds.height;
                    float normalizedT = 2.0f * t - 1.0f; float curveAmount = 1.0f - normalizedT * normalizedT;
                    float x = (leftX - curveOffset) + curveDepth * curveAmount; Vector2 point = {x, y};
                    DrawLineEx(prevPoint, point, 1.0f, BLACK); prevPoint = point;
                }
            } break;
        case GateType::NOT: {
                float bubbleRadius = 3.0f; float triangleHeight = bounds.height; float idealWidth = triangleHeight * 0.866f;
                float actualWidth = fminf(bounds.width - bubbleRadius * 2, idealWidth);
                float leftX = bounds.x + (bounds.width - actualWidth - bubbleRadius * 2) / 2.0f;
                Vector2 points[3] = { {leftX, bounds.y}, {leftX, bounds.y + bounds.height}, {leftX + actualWidth, bounds.y + bounds.height/2} };
                DrawTriangle(points[0], points[1], points[2], color);
                DrawLineEx(points[0], points[1], 1.0f, BLACK); DrawLineEx(points[1], points[2], 1.0f, BLACK); DrawLineEx(points[2], points[0], 1.0f, BLACK);
                Vector2 bubblePos = {leftX + actualWidth + bubbleRadius, bounds.y + bounds.height/2};
                DrawCircleV(bubblePos, bubbleRadius, color); DrawCircleLines(bubblePos.x, bubblePos.y, bubbleRadius, BLACK);
            } break;
        default:
            DrawRectangleRec(bounds, color);
            DrawRectangleLinesEx(bounds, 1.0f, BLACK);
            break;
    }
}
