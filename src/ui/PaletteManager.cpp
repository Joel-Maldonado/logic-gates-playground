#include "ui/PaletteManager.h"
#include "ui/VisualEffects.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "core/DerivedGates.h"
#include <utility>

PaletteManager::PaletteManager(std::shared_ptr<CircuitSimulator> sim)
    : selectedGateType_(GateType::NONE), simulator_(sim),
      isDraggingGate_(false), draggedGateType_(GateType::NONE),
      dragStartPos_({0, 0}), currentDragPos_({0, 0}) {
}

void PaletteManager::initialize() {
    float currentY = Config::CANVAS_PADDING;
    GateType types[] = {
        GateType::INPUT_SOURCE, GateType::OUTPUT_SINK,
        GateType::AND, GateType::OR, GateType::XOR, GateType::NOT
    };

    for (GateType type : types) {
        gatePalette_.push_back({
            {
                Config::CANVAS_PADDING,
                currentY,
                Config::PALETTE_WIDTH - 2 * Config::CANVAS_PADDING,
                Config::PALETTE_ITEM_HEIGHT
            },
            type,
            getGateTypeName(type)
        });
        currentY += Config::PALETTE_ITEM_HEIGHT + Config::CANVAS_PADDING;
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

    // Check for hover state
    Vector2 mousePos = GetMousePosition();

    // Draw palette items with enhanced styling
    for (const auto& item : gatePalette_) {
        bool isHovered = CheckCollisionPointRec(mousePos, item.bounds);
        bool isSelected = (selectedGateType_ == item.type);

        Color bgColor;
        if (isSelected) {
            bgColor = Config::Colors::PALETTE_ITEM_SELECTED;
        } else if (isHovered) {
            bgColor = Config::Colors::PALETTE_ITEM_HOVER;
        } else {
            bgColor = Config::Colors::PALETTE_ITEM;
        }

        // Draw item background with shadow and gradient
        Vector2 shadowOffset = {2.0f, 2.0f};
        VisualEffects::drawRoundedRectangleWithShadow(item.bounds, 0.3f, bgColor,
                                                    Config::Colors::GATE_SHADOW, shadowOffset);

        // Draw selection/hover indicator with glow
        if (isSelected) {
            Color glowColor = Fade(Config::Colors::SELECTION_HIGHLIGHT, 0.3f);
            Rectangle expandedBounds = {
                item.bounds.x - 2.0f, item.bounds.y - 2.0f,
                item.bounds.width + 4.0f, item.bounds.height + 4.0f
            };
            DrawRectangleRounded(expandedBounds, 0.3f, 8, glowColor);
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::SELECTION_HIGHLIGHT);
        } else if (isHovered) {
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::PRIMARY_VARIANT);
        } else {
            DrawRectangleRoundedLines(item.bounds, 0.3f, 8, Config::Colors::GATE_OUTLINE);
        }

        // Draw gate icon
        Rectangle iconBounds = {
            item.bounds.x + 10,
            item.bounds.y + (item.bounds.height - 35) / 2, // Increased from 30 to 35
            35,                                           // Increased from 30 to 35
            35                                            // Increased from 30 to 35
        };

        Color iconColor = getGateColor(item.type);
        drawGateIcon(item.type, iconBounds, iconColor);

        // Draw gate label
        Vector2 textSize = MeasureTextEx(GetFontDefault(), item.label.c_str(), 22, 1);
        DrawText(
            item.label.c_str(),
            item.bounds.x + 55 + (item.bounds.width - 55 - textSize.x) / 2, // Adjusted from 50 to 55 to account for larger icon
            item.bounds.y + (item.bounds.height - textSize.y) / 2,
            22,
            Config::Colors::PALETTE_TEXT
        );
    }

    // Draw instructions at the bottom with more space and updated controls
    DrawRectangle(0, GetScreenHeight() - 80, Config::PALETTE_WIDTH, 80, Fade(BLACK, 0.5f));
    DrawText("Drag: Place", 10, GetScreenHeight() - 70, 18, LIGHTGRAY);
    DrawText("Middle/Right: Pan", 10, GetScreenHeight() - 50, 18, LIGHTGRAY);
    DrawText("Backspace: Remove", 10, GetScreenHeight() - 30, 18, LIGHTGRAY);

    // Draw gate preview if dragging
    if (isDraggingGate_ && draggedGateType_ != GateType::NONE) {
        // Get the world position for the mouse
        Vector2 worldPos = GetScreenToWorld2D(currentDragPos_, camera);

        // Convert back to screen coordinates for drawing
        Vector2 screenPos = GetWorldToScreen2D(worldPos, camera);

        // Draw a preview of the gate at the position
        Rectangle previewBounds = {
            screenPos.x - Config::DRAG_PREVIEW_SIZE / 2,
            screenPos.y - Config::DRAG_PREVIEW_SIZE / 2,
            Config::DRAG_PREVIEW_SIZE,
            Config::DRAG_PREVIEW_SIZE
        };

        Color previewColor = getGateColor(draggedGateType_);
        previewColor.a = static_cast<unsigned char>(255 * Config::DRAG_PREVIEW_ALPHA);

        drawGateIcon(draggedGateType_, previewBounds, previewColor);

        // Draw a "+" symbol to indicate placement
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
            // Instead of just selecting, we'll highlight it but not set as selected
            // This allows us to distinguish between selection and dragging
            selectedGateType_ = item.type;
            return true;
        }
    }

    return false;
}

std::unique_ptr<LogicGate> PaletteManager::createSelectedGateInstance(Vector2 position) {
    return createGateInstance(selectedGateType_, position);
}

GateType PaletteManager::getSelectedGateType() const {
    return selectedGateType_;
}

void PaletteManager::setSelectedGateType(GateType type) {
    selectedGateType_ = type;
}

std::string PaletteManager::getGateTypeName(GateType type) {
    switch (type) {
        case GateType::INPUT_SOURCE: return "Input";
        case GateType::OUTPUT_SINK:  return "Output";
        case GateType::AND:          return "AND";
        case GateType::OR:           return "OR";
        case GateType::XOR:          return "XOR";
        case GateType::NOT:          return "NOT";
        default:                     return "Unknown";
    }
}

Rectangle PaletteManager::getPaletteBounds() const {
    return {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()};
}

void PaletteManager::handleWindowResize() {
    // No need to update the palette items' positions as they are fixed
    // The palette will automatically extend to the new window height
}

bool PaletteManager::startDraggingGate(Vector2 mousePos) {
    Rectangle paletteBounds = {0, 0, Config::PALETTE_WIDTH, (float)GetScreenHeight()};

    if (!CheckCollisionPointRec(mousePos, paletteBounds)) {
        return false;
    }

    for (const auto& item : gatePalette_) {
        if (CheckCollisionPointRec(mousePos, item.bounds)) {
            isDraggingGate_ = true;
            draggedGateType_ = item.type;
            dragStartPos_ = mousePos;
            currentDragPos_ = mousePos;
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
    if (!isDraggingGate_ || draggedGateType_ == GateType::NONE) {
        return nullptr;
    }

    // Create the gate at the world position
    auto newGate = createGateInstance(draggedGateType_, worldPos);
    LogicGate* rawPtr = nullptr;

    if (newGate) {
        rawPtr = simulator_->addGate(std::move(newGate));
    }

    // Reset dragging state
    isDraggingGate_ = false;
    draggedGateType_ = GateType::NONE;

    return rawPtr;
}

void PaletteManager::cancelDraggingGate() {
    isDraggingGate_ = false;
    draggedGateType_ = GateType::NONE;
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



// Helper method to create a gate instance of a specific type
std::unique_ptr<LogicGate> PaletteManager::createGateInstance(GateType type, Vector2 position) {
    if (type == GateType::NONE) {
        return nullptr;
    }

    std::string idStr = "gate" + std::to_string(simulator_->useNextGateId());

    switch (type) {
        case GateType::INPUT_SOURCE:
            return std::make_unique<InputSource>(
                idStr,
                position,
                Vector2{Config::INPUT_OUTPUT_SIZE, Config::INPUT_OUTPUT_SIZE},
                "IN"
            );

        case GateType::OUTPUT_SINK:
            return std::make_unique<OutputSink>(
                idStr,
                position,
                Config::INPUT_OUTPUT_SIZE / 2,
                "OUT"
            );

        case GateType::AND:
            return std::make_unique<AndGate>(
                idStr,
                position,
                Config::DEFAULT_GATE_WIDTH,
                Config::DEFAULT_GATE_HEIGHT
            );

        case GateType::OR:
            return std::make_unique<OrGate>(
                idStr,
                position,
                Config::DEFAULT_GATE_WIDTH,
                Config::DEFAULT_GATE_HEIGHT
            );

        case GateType::XOR:
            return std::make_unique<XorGate>(
                idStr,
                position,
                Config::DEFAULT_GATE_WIDTH,
                Config::DEFAULT_GATE_HEIGHT
            );

        case GateType::NOT:
            return std::make_unique<NotGate>(
                idStr,
                position,
                Config::DEFAULT_GATE_WIDTH,
                Config::DEFAULT_GATE_HEIGHT
            );

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
        default:                     return Config::Colors::GATE_FILL;
    }
}

void PaletteManager::drawGateIcon(GateType type, Rectangle bounds, Color color) const {
    // Draw a simplified icon for each gate type
    switch (type) {
        case GateType::INPUT_SOURCE:
            // Draw a square for input
            DrawRectangleRounded(bounds, 0.3f, 4, color);
            DrawRectangleRoundedLines(bounds, 0.3f, 4, BLACK);
            break;

        case GateType::OUTPUT_SINK:
            // Draw a circle
            {
                Vector2 center = {bounds.x + bounds.width/2, bounds.y + bounds.height/2};
                float radius = bounds.width/2 - 2;
                DrawCircleV(center, radius, color);
                DrawCircleLines(center.x, center.y, radius, BLACK);
            }
            break;

        case GateType::AND:
            // Draw a proper AND gate (D shape)
            {
                // Calculate dimensions for a proper D-shape
                float radius = bounds.height / 2.0f;
                float flatPartWidth = bounds.width * 0.6f; // Left flat part width
                Vector2 center = {bounds.x + flatPartWidth, bounds.y + bounds.height/2};

                // Draw the D-shape using triangles
                int segments = 20;

                // Draw the rectangle part (left side) as a single filled rectangle
                Rectangle rectPart = {
                    bounds.x,
                    bounds.y,
                    flatPartWidth,
                    bounds.height
                };
                DrawRectangleRec(rectPart, color);

                // Draw the semicircle part using triangles
                for (int i = 0; i < segments; i++) {
                    float startAngle = PI/2 - (i * PI / segments);
                    float endAngle = PI/2 - ((i + 1) * PI / segments);

                    Vector2 p1 = center;
                    Vector2 p2 = {
                        center.x + radius * cosf(startAngle),
                        center.y + radius * sinf(startAngle)
                    };
                    Vector2 p3 = {
                        center.x + radius * cosf(endAngle),
                        center.y + radius * sinf(endAngle)
                    };

                    DrawTriangle(p1, p2, p3, color);
                }

                // Draw the outline
                // Left vertical line
                DrawLineEx({bounds.x, bounds.y}, {bounds.x, bounds.y + bounds.height}, 1.0f, BLACK);

                // Top horizontal line to semicircle
                DrawLineEx({bounds.x, bounds.y}, {center.x, bounds.y}, 1.0f, BLACK);

                // Bottom horizontal line to semicircle
                DrawLineEx({bounds.x, bounds.y + bounds.height}, {center.x, bounds.y + bounds.height}, 1.0f, BLACK);

                // Draw the semicircle outline using line segments
                for (int i = 0; i < segments; i++) {
                    float startAngle = PI/2 - (i * PI / segments);
                    float endAngle = PI/2 - ((i + 1) * PI / segments);

                    Vector2 p1 = {
                        center.x + radius * cosf(startAngle),
                        center.y + radius * sinf(startAngle)
                    };
                    Vector2 p2 = {
                        center.x + radius * cosf(endAngle),
                        center.y + radius * sinf(endAngle)
                    };

                    DrawLineEx(p1, p2, 1.0f, BLACK);
                }
            }
            break;

        case GateType::OR:
            // Draw a proper OR gate (triangular/shield-like shape)
            {
                // Make the gate narrower for better proportions
                float actualWidth = bounds.width * 0.85f;
                float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
                float rightX = leftX + actualWidth;

                // Calculate points for the triangular shape
                Vector2 rightPoint = {rightX, bounds.y + bounds.height/2};

                // Calculate curve parameters for the left (input) side
                const int segments = 10; // Fewer segments for the icon
                float curveDepth = bounds.height * 0.2f; // Increase curve depth for more pronounced curve

                // Create points for the curved left side
                Vector2 curvePoints[segments + 1];

                // Generate the curve points
                for (int i = 0; i <= segments; i++) {
                    float t = (float)i / segments;
                    float y = bounds.y + t * bounds.height;

                    // Calculate x with a slight curve (parabolic)
                    float normalizedT = 2.0f * t - 1.0f; // -1 to 1
                    float curveAmount = 1.0f - normalizedT * normalizedT; // Parabolic curve
                    float x = leftX + curveDepth * curveAmount;

                    curvePoints[i] = {x, y};
                }

                // Draw filled triangles to create the OR gate shape
                for (int i = 0; i < segments; i++) {
                    DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, color);
                }

                // Draw the curved left side outline
                for (int i = 0; i < segments; i++) {
                    DrawLineEx(curvePoints[i], curvePoints[i+1], 1.0f, BLACK);
                }

                // Draw the straight lines from curve endpoints to the right point
                DrawLineEx(curvePoints[0], rightPoint, 1.0f, BLACK);
                DrawLineEx(curvePoints[segments], rightPoint, 1.0f, BLACK);
            }
            break;

        case GateType::XOR:
            // Draw a proper XOR gate (OR with extra curve)
            {
                // Make the gate narrower for better proportions
                float actualWidth = bounds.width * 0.8f;
                float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
                float rightX = leftX + actualWidth;

                // Calculate points for the triangular shape
                Vector2 rightPoint = {rightX, bounds.y + bounds.height/2};

                // Calculate curve parameters for the left (input) side
                const int segments = 10; // Fewer segments for the icon
                float curveDepth = bounds.height * 0.2f; // Increase curve depth for more pronounced curve
                float curveOffset = bounds.height * 0.2f; // Offset for the second curve

                // Create points for the curved left side
                Vector2 curvePoints[segments + 1];

                // Generate the curve points
                for (int i = 0; i <= segments; i++) {
                    float t = (float)i / segments;
                    float y = bounds.y + t * bounds.height;

                    // Calculate x with a slight curve (parabolic)
                    float normalizedT = 2.0f * t - 1.0f; // -1 to 1
                    float curveAmount = 1.0f - normalizedT * normalizedT; // Parabolic curve
                    float x = leftX + curveDepth * curveAmount;

                    curvePoints[i] = {x, y};
                }

                // Draw filled triangles to create the OR gate shape
                for (int i = 0; i < segments; i++) {
                    DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, color);
                }

                // Draw the curved left side outline
                for (int i = 0; i < segments; i++) {
                    DrawLineEx(curvePoints[i], curvePoints[i+1], 1.0f, BLACK);
                }

                // Draw the straight lines from curve endpoints to the right point
                DrawLineEx(curvePoints[0], rightPoint, 1.0f, BLACK);
                DrawLineEx(curvePoints[segments], rightPoint, 1.0f, BLACK);

                // Draw the second curved line (XOR's distinctive double-curve)
                Vector2 prevPoint = {leftX - curveOffset, bounds.y}; // Top point of second curve

                for (int i = 1; i <= segments; i++) {
                    float t = (float)i / segments;
                    float y = bounds.y + t * bounds.height;

                    // Calculate x with a slight curve (parabolic)
                    float normalizedT = 2.0f * t - 1.0f; // -1 to 1
                    float curveAmount = 1.0f - normalizedT * normalizedT; // Parabolic curve
                    float x = (leftX - curveOffset) + curveDepth * curveAmount;

                    Vector2 point = {x, y};

                    // Draw line segment of the curve
                    DrawLineEx(prevPoint, point, 1.0f, BLACK);
                    prevPoint = point;
                }
            }
            break;

        case GateType::NOT:
            // Draw a proper NOT gate (triangle with bubble)
            {
                float bubbleRadius = 3.0f;

                // Calculate dimensions for a more equilateral triangle
                float triangleHeight = bounds.height;
                float idealWidth = triangleHeight * 0.866f; // width = height * sqrt(3)/2 for equilateral

                float actualWidth = fminf(bounds.width - bubbleRadius * 2, idealWidth);
                float leftX = bounds.x + (bounds.width - actualWidth - bubbleRadius * 2) / 2.0f;

                // Calculate triangle points
                Vector2 points[3] = {
                    {leftX, bounds.y},                // Top-left
                    {leftX, bounds.y + bounds.height}, // Bottom-left
                    {leftX + actualWidth, bounds.y + bounds.height/2}    // Right-middle
                };

                // Draw filled triangle
                DrawTriangle(points[0], points[1], points[2], color);

                // Draw outline
                DrawLineEx(points[0], points[1], 1.0f, BLACK);
                DrawLineEx(points[1], points[2], 1.0f, BLACK);
                DrawLineEx(points[2], points[0], 1.0f, BLACK);

                // NOT bubble - position it right at the triangle tip
                Vector2 bubblePos = {leftX + actualWidth + bubbleRadius, bounds.y + bounds.height/2};
                DrawCircleV(bubblePos, bubbleRadius, color);
                DrawCircleLines(bubblePos.x, bubblePos.y, bubbleRadius, BLACK);
            }
            break;







        default:
            // Draw a generic rectangle
            DrawRectangleRec(bounds, color);
            DrawRectangleLinesEx(bounds, 1.0f, BLACK);
            break;
    }
}
