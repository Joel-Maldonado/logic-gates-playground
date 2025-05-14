#include "ui/GateRenderer.h"
#include "app/Config.h"
#include "raymath.h"
#include "core/DerivedGates.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include <typeinfo>
#include <string>

GateRenderer::GateRenderer() {
}

void GateRenderer::renderGates(const std::vector<std::unique_ptr<LogicGate>>& gates, const Camera2D& camera) {
    for (const auto& gate : gates) {
        renderGate(gate.get());

        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
        renderPinHighlights(gate.get(), mousePos, false, nullptr);
    }
}

void GateRenderer::renderGate(const LogicGate* gate) {
    if (!gate) {
        return;
    }

    GateType gateType = determineGateType(gate);
    renderGateBody(gate, gateType);
    renderGatePins(gate);
}

void GateRenderer::renderPinHighlights(const LogicGate* gate, Vector2 mousePos, bool isDrawingWire, const GatePin* wireStartPin) {
    if (!gate) {
        return;
    }

    if (isDrawingWire && wireStartPin && wireStartPin->getParentGate() == gate) {
        return;
    }

    for (size_t i = 0; i < gate->getInputPinCount(); i++) {
        const GatePin* pin = gate->getInputPin(i);
        if (pin && pin->isMouseOverPin(mousePos)) {
            DrawCircleV(pin->getAbsolutePosition(), pin->getClickRadius() + 2.0f, Fade(Config::Colors::PIN_HOVER, 0.5f));
        }
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); i++) {
        const GatePin* pin = gate->getOutputPin(i);
        if (pin && pin->isMouseOverPin(mousePos)) {
            DrawCircleV(pin->getAbsolutePosition(), pin->getClickRadius() + 2.0f, Fade(Config::Colors::PIN_HOVER, 0.5f));
        }
    }
}

GateType GateRenderer::determineGateType(const LogicGate* gate) const {
    if (!gate) {
        return GateType::NONE;
    }

    // Identify gate type using dynamic_cast to check inheritance hierarchy
    if (dynamic_cast<const InputSource*>(gate)) {
        return GateType::INPUT_SOURCE;
    } else if (dynamic_cast<const OutputSink*>(gate)) {
        return GateType::OUTPUT_SINK;
    } else if (dynamic_cast<const AndGate*>(gate)) {
        return GateType::AND;
    } else if (dynamic_cast<const OrGate*>(gate)) {
        return GateType::OR;
    } else if (dynamic_cast<const XorGate*>(gate)) {
        return GateType::XOR;
    } else if (dynamic_cast<const NotGate*>(gate)) {
        return GateType::NOT;
    } else if (dynamic_cast<const NandGate*>(gate)) {
        return GateType::NAND;
    } else if (dynamic_cast<const NorGate*>(gate)) {
        return GateType::NOR;
    } else if (dynamic_cast<const XnorGate*>(gate)) {
        return GateType::XNOR;
    }

    return GateType::NONE;
}

void GateRenderer::renderGateBody(const LogicGate* gate, GateType type) const {
    if (!gate) {
        return;
    }

    Rectangle bounds = gate->getBounds();
    float outlineThickness = gate->getIsSelected() ?
                            Config::GATE_OUTLINE_THICKNESS_SELECTED :
                            Config::GATE_OUTLINE_THICKNESS;
    Color outlineColor = gate->getIsSelected() ?
                        Config::Colors::SELECTION_HIGHLIGHT :
                        Config::Colors::GATE_OUTLINE;
    Color fillColor;

    switch (type) {
        case GateType::INPUT_SOURCE:
        {
            const InputSource* inputSource = dynamic_cast<const InputSource*>(gate);
            if (inputSource && inputSource->getCurrentState()) {
                fillColor = Config::Colors::INPUT_ON;
            } else {
                fillColor = Config::Colors::INPUT_OFF;
            }

            // Draw input source as a square with rounded corners
            DrawRectangleRounded(bounds, 0.3f, 8, fillColor);
            DrawRectangleRoundedLines(bounds, 0.3f, 8, outlineColor);

            // Draw label
            const char* stateText = (inputSource && inputSource->getCurrentState()) ? "1" : "0";
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 1.5f;
            Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
            Vector2 textPos = {
                bounds.x + (bounds.width - textSize.x) / 2.0f,
                bounds.y + (bounds.height - textSize.y) / 2.0f
            };
            DrawTextEx(GetFontDefault(), stateText, textPos, fontSize, 1.0f, Config::Colors::IO_TEXT);

            // Draw "IN" label above
            Vector2 labelSize = MeasureTextEx(GetFontDefault(), "IN", Config::GATE_LABEL_FONT_SIZE, 1.0f);
            Vector2 labelPos = {
                bounds.x + (bounds.width - labelSize.x) / 2.0f,
                bounds.y - labelSize.y - 5.0f
            };
            DrawTextEx(GetFontDefault(), "IN", labelPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::IO_TEXT);

            return;
        }
        case GateType::OUTPUT_SINK:
        {
            const OutputSink* outputSink = dynamic_cast<const OutputSink*>(gate);
            if (outputSink && outputSink->isActive()) {
                fillColor = Config::Colors::OUTPUT_ON;
            } else {
                fillColor = Config::Colors::OUTPUT_OFF;
            }

            // Draw output sink as a circle
            Vector2 center = {
                bounds.x + bounds.width / 2.0f,
                bounds.y + bounds.height / 2.0f
            };
            float radius = bounds.width / 2.0f;

            DrawCircleV(center, radius, fillColor);
            DrawCircleLines(center.x, center.y, radius, outlineColor);
            if (gate->getIsSelected()) {
                DrawCircleLines(center.x, center.y, radius + 2.0f, outlineColor);
            }

            // Draw state
            const char* stateText = (outputSink && outputSink->isActive()) ? "1" : "0";
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 1.5f;
            Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
            Vector2 textPos = {
                center.x - textSize.x / 2.0f,
                center.y - textSize.y / 2.0f
            };
            DrawTextEx(GetFontDefault(), stateText, textPos, fontSize, 1.0f, Config::Colors::IO_TEXT);

            // Draw "OUT" label above
            Vector2 labelSize = MeasureTextEx(GetFontDefault(), "OUT", Config::GATE_LABEL_FONT_SIZE, 1.0f);
            Vector2 labelPos = {
                center.x - labelSize.x / 2.0f,
                bounds.y - labelSize.y - 5.0f
            };
            DrawTextEx(GetFontDefault(), "OUT", labelPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::IO_TEXT);

            return;
        }
        case GateType::AND:
            fillColor = Config::Colors::AND_GATE;
            renderAndGateSymbol(bounds, fillColor, outlineColor, outlineThickness);
            break;
        case GateType::OR:
            fillColor = Config::Colors::OR_GATE;
            renderOrGateSymbol(bounds, fillColor, outlineColor, outlineThickness);
            break;
        case GateType::XOR:
            fillColor = Config::Colors::XOR_GATE;
            renderXorGateSymbol(bounds, fillColor, outlineColor, outlineThickness);
            break;
        case GateType::NOT:
            fillColor = Config::Colors::NOT_GATE;
            renderNotGateSymbol(bounds, fillColor, outlineColor, outlineThickness);
            break;
        case GateType::NAND:
        {
            fillColor = Config::Colors::NAND_GATE;

            // Use the full bounds for the AND part since we're removing the bubble
            Rectangle andBounds = bounds;

            // Draw the AND gate
            renderAndGateSymbol(andBounds, fillColor, outlineColor, outlineThickness);

            // Update the label
            const char* label = "NAND";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
            Vector2 textPos = {
                andBounds.x + (andBounds.width - textSize.x) / 2.0f,
                andBounds.y + (andBounds.height - textSize.y) / 2.0f
            };

            // Clear the area where the AND text was drawn
            DrawRectangle(textPos.x - 2, textPos.y - 2, textSize.x + 4, textSize.y + 4, fillColor);

            // Draw the NAND text
            DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
            break;
        }
        case GateType::NOR:
        {
            fillColor = Config::Colors::NOR_GATE;

            // Use the full bounds for the OR part since we're removing the bubble
            Rectangle orBounds = bounds;

            // Draw the OR gate
            renderOrGateSymbol(orBounds, fillColor, outlineColor, outlineThickness);

            // Update the label
            const char* label = "NOR";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);

            // Use the same approach as OR and XOR for consistency
            float actualWidth = orBounds.width * 0.8f; // Same as in renderOrGateSymbol
            float leftX = orBounds.x + (orBounds.width - actualWidth) / 2.0f;

            // Calculate a better centered position for the text
            float centerX = leftX + actualWidth * 0.4f; // Position at 40% of the gate width from the left
            Vector2 textPos = {
                centerX - textSize.x / 2.0f, // Center horizontally at the calculated position
                orBounds.y + (orBounds.height - textSize.y) / 2.0f // Center vertically
            };

            // Clear the area where the OR text was drawn with a properly sized background
            DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);

            // Draw the NOR text
            DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
            break;
        }
        case GateType::XNOR:
        {
            fillColor = Config::Colors::XNOR_GATE;

            // Use the full bounds for the XOR part since we're removing the bubble
            Rectangle xorBounds = bounds;

            // Draw the XOR gate
            renderXorGateSymbol(xorBounds, fillColor, outlineColor, outlineThickness);

            // Update the label - use a smaller font size for XNOR to fit better
            const char* label = "XNOR"; // Make sure this is exactly "XNOR" with no extra characters
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 0.85f; // Smaller font for XNOR
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, fontSize, 1.0f);

            // Use the same approach as XOR for consistency
            float actualWidth = xorBounds.width * 0.8f; // Same as in renderOrGateSymbol
            float leftX = xorBounds.x + (xorBounds.width - actualWidth) / 2.0f;

            // Calculate a better centered position for the text
            float centerX = leftX + actualWidth * 0.4f; // Position at 40% of the gate width from the left
            Vector2 textPos = {
                centerX - textSize.x / 2.0f, // Center horizontally at the calculated position
                xorBounds.y + (xorBounds.height - textSize.y) / 2.0f // Center vertically
            };

            // Draw the background with minimal padding to avoid clipping
            // This ensures it stays completely within the gate body
            DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);

            // Draw the XNOR text
            DrawTextEx(GetFontDefault(), label, textPos, fontSize, 1.0f, Config::Colors::GATE_TEXT);
            break;
        }
        default:
            // Fallback to a generic rectangle
            fillColor = Config::Colors::GATE_FILL;
            DrawRectangleRec(bounds, fillColor);
            DrawRectangleLinesEx(bounds, outlineThickness, outlineColor);

            // Draw gate ID
            DrawTextEx(GetFontDefault(), gate->getId().c_str(),
                      {bounds.x + 5.0f, bounds.y + 5.0f},
                      Config::GATE_LABEL_FONT_SIZE, 1.0f,
                      Config::Colors::GATE_TEXT);
    }
}

void GateRenderer::renderGatePins(const LogicGate* gate) const {
    if (!gate) {
        return;
    }

    // Only show state labels on input/output gates
    bool isIOGate = (dynamic_cast<const InputSource*>(gate) != nullptr) ||
                    (dynamic_cast<const OutputSink*>(gate) != nullptr);

    for (size_t i = 0; i < gate->getInputPinCount(); i++) {
        const GatePin* pin = gate->getInputPin(i);
        if (pin) {
            renderPin(pin, isIOGate);
        }
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); i++) {
        const GatePin* pin = gate->getOutputPin(i);
        if (pin) {
            renderPin(pin, isIOGate);
        }
    }
}

void GateRenderer::renderPin(const GatePin* pin, bool showLabel) const {
    if (!pin) {
        return;
    }

    Vector2 pos = pin->getAbsolutePosition();
    Color pinColor = pin->getState() ? Config::Colors::PIN_STATE_ON : Config::Colors::PIN_STATE_OFF;

    // Draw pin circle
    DrawCircleV(pos, pin->getClickRadius(), pinColor);
    DrawCircleLines(pos.x, pos.y, pin->getClickRadius(), Config::Colors::GATE_OUTLINE);

    // Draw pin state label if requested
    if (showLabel) {
        const char* stateText = pin->getState() ? "1" : "0";
        float fontSize = Config::PIN_LABEL_FONT_SIZE;

        // Position the label based on pin type
        Vector2 labelPos;
        if (pin->getType() == PinType::INPUT_PIN) {
            labelPos = {pos.x - Config::PIN_LABEL_OFFSET, pos.y - fontSize / 2.0f};
        } else {
            labelPos = {pos.x + Config::PIN_LABEL_OFFSET / 2.0f, pos.y - fontSize / 2.0f};
        }

        DrawTextEx(GetFontDefault(), stateText, labelPos, fontSize, 1.0f, Config::Colors::PIN_TEXT);
    }
}

void GateRenderer::renderAndGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create a D-shaped AND gate with flat left side and semicircle on right
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};
    float radius = bounds.height / 2.0f;
    float flatPartWidth = bounds.width - radius;
    int segments = 20; // Number of segments to approximate the semicircle

    // Draw the rectangle part (left side)
    Rectangle rectPart = {
        bounds.x,
        bounds.y,
        flatPartWidth,
        bounds.height
    };
    DrawRectangleRec(rectPart, fillColor);

    // Draw the semicircle part using triangles for smooth appearance
    Vector2 semicircleCenter = {bounds.x + flatPartWidth, center.y};
    for (int i = 0; i < segments; i++) {
        float startAngle = PI/2 - (i * PI / segments);
        float endAngle = PI/2 - ((i + 1) * PI / segments);

        Vector2 p1 = semicircleCenter;
        Vector2 p2 = {
            semicircleCenter.x + radius * cosf(startAngle),
            semicircleCenter.y + radius * sinf(startAngle)
        };
        Vector2 p3 = {
            semicircleCenter.x + radius * cosf(endAngle),
            semicircleCenter.y + radius * sinf(endAngle)
        };

        DrawTriangle(p1, p2, p3, fillColor);
    }

    // Draw the outline
    DrawLineEx({bounds.x, bounds.y}, {bounds.x + flatPartWidth, bounds.y}, outlineThickness, outlineColor); // Top
    DrawLineEx({bounds.x, bounds.y}, {bounds.x, bounds.y + bounds.height}, outlineThickness, outlineColor); // Left
    DrawLineEx({bounds.x, bounds.y + bounds.height}, {bounds.x + flatPartWidth, bounds.y + bounds.height}, outlineThickness, outlineColor); // Bottom

    // Draw the semicircle outline
    for (int i = 0; i < segments; i++) {
        float startAngle = PI/2 - (i * PI / segments);
        float endAngle = PI/2 - ((i + 1) * PI / segments);

        Vector2 p1 = {
            semicircleCenter.x + radius * cosf(startAngle),
            semicircleCenter.y + radius * sinf(startAngle)
        };
        Vector2 p2 = {
            semicircleCenter.x + radius * cosf(endAngle),
            semicircleCenter.y + radius * sinf(endAngle)
        };

        DrawLineEx(p1, p2, outlineThickness, outlineColor);
    }

    // Draw gate label centered
    const char* label = "AND";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    Vector2 textPos = {
        bounds.x + (bounds.width - textSize.x) / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create shield-like OR gate with curved input side and point on right
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Adjust width for better proportions
    float actualWidth = bounds.width * 0.8f;
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
    float rightX = leftX + actualWidth;
    Vector2 rightPoint = {rightX, center.y};

    // Parameters for the curved left side
    int segments = 20;
    float curveDepth = bounds.height * 0.15f;
    Vector2 curvePoints[segments + 1];

    // Generate parabolic curve points for the left side
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        // Parabolic curve calculation - maximum curve at center, straight at ends
        float normalizedT = 2.0f * t - 1.0f; // -1 to 1
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = leftX + curveDepth * curveAmount;

        curvePoints[i] = {x, y};
    }

    // Draw filled triangles to create the OR gate shape
    for (int i = 0; i < segments; i++) {
        DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, fillColor);
    }

    // Draw the curved left side outline
    for (int i = 0; i < segments; i++) {
        DrawLineEx(curvePoints[i], curvePoints[i+1], outlineThickness, outlineColor);
    }

    // Draw the straight lines from curve endpoints to the right point
    DrawLineEx(curvePoints[0], rightPoint, outlineThickness, outlineColor);
    DrawLineEx(curvePoints[segments], rightPoint, outlineThickness, outlineColor);

    // Draw gate label centered in the visible area (40% from left edge)
    const char* label = "OR";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    float centerX = leftX + actualWidth * 0.4f;
    Vector2 textPos = {
        centerX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    // Background for text visibility
    DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // First draw the OR gate as the base
    renderOrGateSymbol(bounds, fillColor, outlineColor, outlineThickness);

    // Add the second curve that distinguishes XOR from OR
    float actualWidth = bounds.width * 0.8f;
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
    float curveOffset = bounds.height * 0.11f;
    float curveDepth = bounds.height * 0.15f;

    // Draw the second curved line with the same parabolic shape
    int segments = 20;
    Vector2 prevPoint = {leftX - curveOffset, bounds.y}; // Top point of second curve

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        // Parabolic curve calculation
        float normalizedT = 2.0f * t - 1.0f; // -1 to 1
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = (leftX - curveOffset) + curveDepth * curveAmount;

        Vector2 point = {x, y};
        DrawLineEx(prevPoint, point, outlineThickness, outlineColor);
        prevPoint = point;
    }

    // Update the label
    const char* label = "XOR";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    float centerX = leftX + actualWidth * 0.4f;
    Vector2 textPos = {
        centerX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    // Clear existing text from OR gate and draw XOR text
    DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderNotGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create triangular NOT gate
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Calculate dimensions for a more equilateral triangle
    // For an equilateral triangle, the width should be approximately 1.155 times the height
    float triangleHeight = bounds.height;
    float idealWidth = triangleHeight * 0.866f; // width = height * sqrt(3)/2 for equilateral

    // Use 70% of the available width to make it more compact
    float actualWidth = fminf(bounds.width * 0.7f, idealWidth);
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;

    // Define triangle points
    Vector2 points[3] = {
        {leftX, bounds.y},                // Top-left
        {leftX, bounds.y + bounds.height}, // Bottom-left
        {leftX + actualWidth, center.y}    // Right-middle
    };

    // Draw filled triangle and outline
    DrawTriangle(points[0], points[1], points[2], fillColor);
    DrawLineEx(points[0], points[1], outlineThickness, outlineColor);
    DrawLineEx(points[1], points[2], outlineThickness, outlineColor);
    DrawLineEx(points[2], points[0], outlineThickness, outlineColor);

    // Draw gate label centered at 40% from left edge
    const char* label = "NOT";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    float centerX = leftX + actualWidth * 0.4f;
    Vector2 textPos = {
        centerX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    // Background for text visibility
    DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderWirePreview(const GatePin* startPin, Vector2 endPos,
                                    const std::vector<std::unique_ptr<LogicGate>>& gates,
                                    Vector2 mousePos) {
    if (!startPin) {
        return;
    }

    // Find pin under mouse cursor
    GatePin* hoverPin = nullptr;
    for (const auto& gate : gates) {
        for (size_t i = 0; i < gate->getInputPinCount(); i++) {
            const GatePin* pin = gate->getInputPin(i);
            if (pin && pin->isMouseOverPin(mousePos)) {
                hoverPin = const_cast<GatePin*>(pin);
                break;
            }
        }
        if (hoverPin) break;
    }

    // Highlight potential connection pin with appropriate color
    if (hoverPin && hoverPin != startPin && hoverPin->getType() == PinType::INPUT_PIN) {
        Color highlightColor = hoverPin->isConnectedInput() ?
                              Config::Colors::PIN_INVALID_CONNECTION :
                              Config::Colors::PIN_VALID_CONNECTION;

        DrawCircleV(hoverPin->getAbsolutePosition(), hoverPin->getClickRadius() * 1.5f, Fade(highlightColor, 0.5f));
    }
}
