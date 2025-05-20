#include "ui/GateRenderer.h"
#include "app/Config.h"
#include <raymath.h>
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
    renderGatePins(gate, gateType); // Pass gateType
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
            renderAndGateSymbol(bounds, fillColor, outlineColor, outlineThickness); // Draws "AND"

            // Draw "NAND" text, ensuring it's centered like the "AND" text in renderAndGateSymbol
            const char* label = "NAND";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);

            // Calculate positioning based on the D-shape's actual visual center
            // This logic must match renderAndGateSymbol's text centering
            // float radius_calc = bounds.height / 2.0f; // Not needed directly for centering with bounds.width
            // float flatPartWidth_calc = radius_calc; 
            float visualCenterX_calc = bounds.x + bounds.width / 2.0f; // Center in full bounds

            Vector2 textPos = {
                visualCenterX_calc - textSize.x / 2.0f,
                bounds.y + (bounds.height - textSize.y) / 2.0f
            };

            // Clear the area where the AND text was drawn by renderAndGateSymbol.
            // Using the NAND text's position and size with padding is simpler and effective here.
            DrawRectangle(textPos.x - 2, textPos.y - 2, textSize.x + 4, textSize.y + 4, fillColor); // Use fillColor for background

            // Draw the NAND text
            DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);

            // Inversion bubble will be drawn by renderGatePins for the output pin
            break;
        }
        case GateType::NOR:
        {
            fillColor = Config::Colors::NOR_GATE;
            renderOrGateSymbol(bounds, fillColor, outlineColor, outlineThickness); // Draws "OR"

            // Draw "NOR" text, ensuring it's centered like the "OR" text in renderOrGateSymbol
            const char* label = "NOR";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);

            // Calculate positioning based on the OR shape's actual visual center
            // This logic must match renderOrGateSymbol's text centering
            // float actualWidth_calc = bounds.width * 0.8f; 
            // float leftX_calc = bounds.x + (bounds.width - actualWidth_calc) / 2.0f; 
            float visualCenterX_calc = bounds.x + bounds.width / 2.0f; // Center in full bounds

            Vector2 textPos = {
                visualCenterX_calc - textSize.x / 2.0f,
                bounds.y + (bounds.height - textSize.y) / 2.0f
            };

            // Clear the area where the OR text was drawn by renderOrGateSymbol.
            // Using the NOR text's position and size with padding is simpler and effective here.
            DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor); // Use fillColor for background

            // Draw the NOR text
            DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);

            // Inversion bubble will be drawn by renderGatePins for the output pin
            break;
        }
        case GateType::XNOR:
        {
            fillColor = Config::Colors::XNOR_GATE;
            renderXorGateSymbol(bounds, fillColor, outlineColor, outlineThickness); // Draws "XOR"

            // Draw "XNOR" text, ensuring it's centered like the "XOR" text in renderXorGateSymbol
            const char* label = "XNOR";
            // Preserve smaller font size for XNOR
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 0.85f; 
            Vector2 textSize = MeasureTextEx(GetFontDefault(), label, fontSize, 1.0f);

            // Calculate positioning based on the XOR shape's actual visual center
            // This logic must match renderXorGateSymbol's text centering
            // float actualWidth_calc = bounds.width * 0.8f;
            // float leftX_calc = bounds.x + (bounds.width - actualWidth_calc) / 2.0f;
            float visualCenterX_calc = bounds.x + bounds.width / 2.0f; // Center in full bounds

            Vector2 textPos = {
                visualCenterX_calc - textSize.x / 2.0f,
                bounds.y + (bounds.height - textSize.y) / 2.0f
            };

            // Clear the area where the XOR text was drawn by renderXorGateSymbol.
            // Using the XNOR text's position and size with padding is simpler and effective here.
            DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor); // Use fillColor for background

            // Draw the XNOR text
            DrawTextEx(GetFontDefault(), label, textPos, fontSize, 1.0f, Config::Colors::GATE_TEXT);
            
            // Inversion bubble will be drawn by renderGatePins for the output pin
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

void GateRenderer::renderGatePins(const LogicGate* gate, GateType gateType) const {
    if (!gate) {
        return;
    }

    // Only show state labels on input/output gates
    bool isIOGate = (dynamic_cast<const InputSource*>(gate) != nullptr) ||
                    (dynamic_cast<const OutputSink*>(gate) != nullptr);

    for (size_t i = 0; i < gate->getInputPinCount(); i++) {
        const GatePin* pin = gate->getInputPin(i);
        if (pin) {
            renderPin(pin, isIOGate, gateType); // Pass gateType
        }
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); i++) {
        const GatePin* pin = gate->getOutputPin(i);
        if (pin) {
            renderPin(pin, isIOGate, gateType); // Pass gateType
        }
    }
}

void GateRenderer::renderPin(const GatePin* pin, bool showLabel, GateType gateType) const {
    if (!pin) {
        return;
    }

    Vector2 pos = pin->getAbsolutePosition();
    constexpr float bubbleRadius = 5.0f; // Consistent with previous usage

    if (pin->getType() == PinType::OUTPUT_PIN &&
        (gateType == GateType::NOT || gateType == GateType::NAND ||
         gateType == GateType::NOR || gateType == GateType::XNOR)) {
        // Draw inversion bubble for output pins of inverted gates
        // Outline color for bubble should be GATE_OUTLINE, fill with BACKGROUND for "hollow" look
        drawInversionBubble(pos, bubbleRadius, Config::Colors::BACKGROUND, Config::Colors::GATE_OUTLINE);
    } else {
        // Draw standard pin
        Color pinColor = pin->getState() ? Config::Colors::PIN_STATE_ON : Config::Colors::PIN_STATE_OFF;
        DrawCircleV(pos, pin->getClickRadius(), pinColor);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), pin->getClickRadius(), Config::Colors::GATE_OUTLINE);
    }

    // Draw pin state label if requested (typically for I/O gates, not standard logic gates)
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
    // Ensure the D-shape uses the full bounds.width.
    float flatPartWidth = bounds.width - radius;
    // Ensure flatPartWidth is not negative if bounds.width is too small
    if (flatPartWidth < 0) flatPartWidth = 0; 
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
    // The effective width of the D-shape is flatPartWidth + radius, which should now be bounds.width.
    // Center the text within bounds.width.
    float visualCenterX = bounds.x + bounds.width / 2.0f;
    Vector2 textPos = {
        visualCenterX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create shield-like OR gate with curved input side and point on right
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Use full bounds.width for the OR gate shape
    // float actualWidth = bounds.width * 0.8f; // Remove this scaling
    // float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f; // This becomes bounds.x
    float leftX = bounds.x;
    // float rightX = leftX + actualWidth; // This becomes bounds.x + bounds.width
    float rightX = bounds.x + bounds.width;
    Vector2 rightPoint = {rightX, center.y};

    // Parameters for the curved left side
    const int segments = 20;
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

    // Draw gate label centered in the visible area
    const char* label = "OR";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    // Center the text within the full bounds.width of the gate shape
    float textCenterX = bounds.x + bounds.width / 2.0f;
    Vector2 textPos = {
        textCenterX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    // Background for text visibility - ensure this doesn't make text look off-center
    // A small, tight background helps.
    DrawRectangleRec(Rectangle{textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2}, fillColor);
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // First draw the OR gate as the base
    renderOrGateSymbol(bounds, fillColor, outlineColor, outlineThickness); // OR symbol now uses full bounds

    // Add the second curve that distinguishes XOR from OR
    // Ensure this curve also scales with the full bounds.width
    // float actualWidth = bounds.width * 0.8f; // Remove this scaling for consistency if curve is relative to gate edge
    // float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f; // This becomes bounds.x
    float leftX = bounds.x; 
    float curveOffset = bounds.height * 0.11f; // Keep curve offset relative to height or a fixed value
    float curveDepth = bounds.height * 0.15f;  // Keep curve depth relative to height or a fixed value


    // Draw the second curved line with the same parabolic shape
    const int segments = 20;
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

    // Update the label for XOR
    const char* label = "XOR";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    // Center the text within the full bounds.width of the gate shape, similar to OR
    float textCenterX = bounds.x + bounds.width / 2.0f; // Centered in full bounds
    Vector2 textPos = {
        textCenterX - textSize.x / 2.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    // Clear existing text (e.g. if OR symbol was drawn first) and draw XOR text
    // The renderOrGateSymbol call already draws "OR". We need to clear that "OR" text first.
    // Position of "OR" text from renderOrGateSymbol (now also centered in full bounds):
    const char* orLabel = "OR"; // Label drawn by renderOrGateSymbol
    Vector2 orTextSize = MeasureTextEx(GetFontDefault(), orLabel, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    float orTextCenterX = bounds.x + bounds.width / 2.0f; 
    Vector2 orTextPos = {
        orTextCenterX - orTextSize.x / 2.0f,
        bounds.y + (bounds.height - orTextSize.y) / 2.0f
    };
    DrawRectangleRec(Rectangle{orTextPos.x -1, orTextPos.y-1, orTextSize.x+2, orTextSize.y+2}, fillColor); // Clear OR text background

    // Now draw XOR text
    DrawRectangleRec(Rectangle{textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2}, fillColor); // Background for XOR text
    DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);
}

void GateRenderer::renderNotGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create triangular NOT gate that fills the bounds
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Triangle points defined by bounds directly
    Vector2 points[3] = {
        {bounds.x, bounds.y},                                     // Top-left
        {bounds.x, bounds.y + bounds.height},                     // Bottom-left
        {bounds.x + bounds.width, bounds.y + bounds.height / 2.0f} // Right-middle
    };

    // Draw filled triangle and outline
    DrawTriangle(points[0], points[1], points[2], fillColor);
    DrawLineEx(points[0], points[1], outlineThickness, outlineColor);
    DrawLineEx(points[1], points[2], outlineThickness, outlineColor);
    DrawLineEx(points[2], points[0], outlineThickness, outlineColor);

    // Draw gate label centered at 40% from left edge
    // const char* label = "NOT"; // Text often omitted for NOT gates
    // Vector2 textSize = MeasureTextEx(GetFontDefault(), label, Config::GATE_LABEL_FONT_SIZE, 1.0f);
    // float centerX = leftX + actualWidth * 0.4f;
    // Vector2 textPos = {
    //     centerX - textSize.x / 2.0f,
    //     bounds.y + (bounds.height - textSize.y) / 2.0f
    // };

    // Background for text visibility
    // DrawRectangle(textPos.x - 1, textPos.y - 1, textSize.x + 2, textSize.y + 2, fillColor);
    // DrawTextEx(GetFontDefault(), label, textPos, Config::GATE_LABEL_FONT_SIZE, 1.0f, Config::Colors::GATE_TEXT);

    // Inversion bubble will be drawn by renderGatePins for the output pin
}

void GateRenderer::drawInversionBubble(Vector2 position, float radius, Color fillColor, Color outlineColor) const {
    DrawCircleV(position, radius, fillColor);
    // DrawCircleLinesV(position, radius, outlineColor); // DrawCircleLinesV takes Vector2 for position
    DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), radius, outlineColor); // Original
    // If thickness from Config::GATE_OUTLINE_THICKNESS is needed, this might need to be a custom ring.
    // For now, standard Raylib circle outline.
}

void GateRenderer::renderWirePreview(const GatePin* startPin,
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
