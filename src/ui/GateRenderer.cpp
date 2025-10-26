#include "ui/GateRenderer.h"
#include "ui/VisualEffects.h"
#include "app/Config.h"
#include <raymath.h>
#include "core/DerivedGates.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include <typeinfo>
#include <string>
#include <cmath>

GateRenderer::GateRenderer() {}

void GateRenderer::renderGates(const std::vector<std::unique_ptr<LogicGate>>& gates, const Camera2D& camera) {
    for (const auto& gate : gates) {
        renderGate(gate.get());

        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
        // Subtle hover indicators (corner ticks) instead of a full rectangle border
        if (gate->isMouseOver(mousePos) && !gate->getIsSelected()) {
            Rectangle b = gate->getBounds();
            float len = 12.0f;
            float th = 2.0f;
            Color hov = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.6f);
            hov.a = (unsigned char)(0.85f * 255);

            // Top-left
            DrawLineEx({b.x, b.y}, {b.x + len, b.y}, th, hov);
            DrawLineEx({b.x, b.y}, {b.x, b.y + len}, th, hov);
            // Top-right
            DrawLineEx({b.x + b.width - len, b.y}, {b.x + b.width, b.y}, th, hov);
            DrawLineEx({b.x + b.width, b.y}, {b.x + b.width, b.y + len}, th, hov);
            // Bottom-left
            DrawLineEx({b.x, b.y + b.height - len}, {b.x, b.y + b.height}, th, hov);
            DrawLineEx({b.x, b.y + b.height}, {b.x + len, b.y + b.height}, th, hov);
            // Bottom-right
            DrawLineEx({b.x + b.width - len, b.y + b.height}, {b.x + b.width, b.y + b.height}, th, hov);
            DrawLineEx({b.x + b.width, b.y + b.height - len}, {b.x + b.width, b.y + b.height}, th, hov);
        }
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
            DrawCircleV(pin->getAbsolutePosition(), pin->getClickRadius() + Config::PIN_HOVER_TOLERANCE, Fade(Config::Colors::PIN_HOVER, 0.5f));
        }
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); i++) {
        const GatePin* pin = gate->getOutputPin(i);
        if (pin && pin->isMouseOverPin(mousePos)) {
            DrawCircleV(pin->getAbsolutePosition(), pin->getClickRadius() + Config::PIN_HOVER_TOLERANCE, Fade(Config::Colors::PIN_HOVER, 0.5f));
        }
    }
}

GateType GateRenderer::determineGateType(const LogicGate* gate) const {
    if (!gate) {
        return GateType::NONE;
    }

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

    // Subtle pulsing effect for selected gates (0.9 to 1.0 range)
    float pulseValue = gate->getIsSelected() ? VisualEffects::getPulseValue() * 0.1f + 0.9f : 1.0f;

    Color fillColor;
    Color fillColorLight;

    switch (type) {
        case GateType::INPUT_SOURCE:
        {
            const InputSource* inputSource = dynamic_cast<const InputSource*>(gate);
            bool isActive = inputSource && inputSource->getCurrentState();

            if (isActive) {
                fillColor = Config::Colors::INPUT_ON;
                fillColorLight = Config::Colors::INPUT_ON;
            } else {
                // Higher contrast off-state body for visibility
                fillColor = VisualEffects::lerpColor(Config::Colors::GATE_FILL, WHITE, 0.12f);
                fillColorLight = VisualEffects::lerpColor(fillColor, WHITE, 0.06f);
            }

            Vector2 shadowOffset = {Config::SHADOW_OFFSET, Config::SHADOW_OFFSET};
            Rectangle shadowBounds = {
                bounds.x + shadowOffset.x,
                bounds.y + shadowOffset.y,
                bounds.width,
                bounds.height
            };
            DrawRectangleRounded(shadowBounds, 0.3f, 8, Config::Colors::GATE_SHADOW);

            DrawRectangleRounded(bounds, 0.3f, 8, fillColor);

            Color currentOutlineColor = outlineColor;
            if (!isActive) {
                currentOutlineColor = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.35f);
            }
            if (gate->getIsSelected()) {
                currentOutlineColor = VisualEffects::lerpColor(outlineColor, Config::Colors::SELECTION_HIGHLIGHT, pulseValue);
            }
            DrawRectangleRoundedLines(bounds, 0.3f, 8, outlineThickness, currentOutlineColor);
            const char* stateText = isActive ? "1" : "0";
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 1.5f;
            Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
            Vector2 textPos = {
                bounds.x + (bounds.width - textSize.x) / 2.0f,
                bounds.y + (bounds.height - textSize.y) / 2.0f
            };

            if (isActive) {
                VisualEffects::drawTextWithShadow(stateText, textPos, fontSize,
                                                Config::Colors::IO_TEXT, Config::Colors::GATE_SHADOW,
                                                {1.0f, 1.0f});
            } else {
                DrawTextEx(GetFontDefault(), stateText, textPos, fontSize, 1.0f, Config::Colors::IO_TEXT);
            }

            Vector2 labelSize = MeasureTextEx(GetFontDefault(), "IN", Config::GATE_LABEL_FONT_SIZE, 1.0f);
            Vector2 labelPos = {
                bounds.x + (bounds.width - labelSize.x) / 2.0f,
                bounds.y - labelSize.y - 5.0f
            };
            VisualEffects::drawTextWithShadow("IN", labelPos, Config::GATE_LABEL_FONT_SIZE,
                                            Config::Colors::IO_TEXT, Config::Colors::GATE_SHADOW,
                                            {1.0f, 1.0f});

            return;
        }
        case GateType::OUTPUT_SINK:
        {
            const OutputSink* outputSink = dynamic_cast<const OutputSink*>(gate);
            bool isActive = outputSink && outputSink->isActive();

            if (isActive) {
                fillColor = Config::Colors::OUTPUT_ON;
                fillColorLight = Config::Colors::OUTPUT_ON;
            } else {
                // Higher contrast off-state knob
                fillColor = VisualEffects::lerpColor(Config::Colors::GATE_FILL, WHITE, 0.15f);
                fillColorLight = VisualEffects::lerpColor(fillColor, WHITE, 0.08f);
            }

            Vector2 center = {
                bounds.x + bounds.width / 2.0f,
                bounds.y + bounds.height / 2.0f
            };
            float radius = bounds.width / 2.0f;

            Vector2 shadowCenter = {center.x + Config::SHADOW_OFFSET, center.y + Config::SHADOW_OFFSET};
            DrawCircleV(shadowCenter, radius, Config::Colors::GATE_SHADOW);

            for (int i = 0; i < 10; i++) {
                float t = (float)i / 9.0f;
                Color gradientColor = VisualEffects::lerpColor(fillColorLight, fillColor, t);
                float currentRadius = radius * (1.0f - t * 0.1f);
                DrawCircleV(center, currentRadius, gradientColor);
            }

            Color currentOutlineColor = outlineColor;
            if (!isActive) {
                currentOutlineColor = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.45f);
            }
            if (gate->getIsSelected()) {
                currentOutlineColor = VisualEffects::lerpColor(outlineColor, Config::Colors::SELECTION_HIGHLIGHT, pulseValue);
            }
            // Use ring for stronger visual separation
            DrawRing(center, radius - 2.0f, radius, 0.0f, 360.0f, 36, currentOutlineColor);
            const char* stateText = isActive ? "1" : "0";
            float fontSize = Config::GATE_LABEL_FONT_SIZE * 1.5f;
            Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
            Vector2 textPos = {
                center.x - textSize.x / 2.0f,
                center.y - textSize.y / 2.0f
            };

            if (isActive) {
                VisualEffects::drawTextWithShadow(stateText, textPos, fontSize,
                                                Config::Colors::IO_TEXT, Config::Colors::GATE_SHADOW,
                                                {1.0f, 1.0f});
            } else {
                // Off-state text slightly dimmed but still legible
                DrawTextEx(GetFontDefault(), stateText, textPos, fontSize, 1.0f,
                           VisualEffects::lerpColor(Config::Colors::IO_TEXT, GRAY, 0.25f));
            }

            Vector2 labelSize = MeasureTextEx(GetFontDefault(), "OUT", Config::GATE_LABEL_FONT_SIZE, 1.0f);
            Vector2 labelPos = {
                center.x - labelSize.x / 2.0f,
                bounds.y - labelSize.y - 5.0f
            };
            VisualEffects::drawTextWithShadow("OUT", labelPos, Config::GATE_LABEL_FONT_SIZE,
                                            Config::Colors::IO_TEXT, Config::Colors::GATE_SHADOW,
                                            {1.0f, 1.0f});

            return;
        }
        case GateType::AND:
            // Use a consistent neutral fill for gate bodies per standard diagrams
            fillColor = Config::Colors::GATE_FILL;
            fillColorLight = Config::Colors::GATE_FILL_LIGHT;
            renderEnhancedGateSymbol(bounds, fillColor, fillColorLight, outlineColor, outlineThickness, pulseValue, gate->getIsSelected(), "AND");
            break;
        case GateType::OR:
            fillColor = Config::Colors::GATE_FILL;
            fillColorLight = Config::Colors::GATE_FILL_LIGHT;
            renderEnhancedGateSymbol(bounds, fillColor, fillColorLight, outlineColor, outlineThickness, pulseValue, gate->getIsSelected(), "OR");
            break;
        case GateType::XOR:
            fillColor = Config::Colors::GATE_FILL;
            fillColorLight = Config::Colors::GATE_FILL_LIGHT;
            renderEnhancedGateSymbol(bounds, fillColor, fillColorLight, outlineColor, outlineThickness, pulseValue, gate->getIsSelected(), "XOR");
            break;
        case GateType::NOT:
            fillColor = Config::Colors::GATE_FILL;
            fillColorLight = Config::Colors::GATE_FILL_LIGHT;
            renderEnhancedGateSymbol(bounds, fillColor, fillColorLight, outlineColor, outlineThickness, pulseValue, gate->getIsSelected(), "NOT");
            break;
        default:
            fillColor = Config::Colors::GATE_FILL;
            DrawRectangleRec(bounds, fillColor);
            DrawRectangleLinesEx(bounds, outlineThickness, outlineColor);

            DrawTextEx(GetFontDefault(), gate->getId().c_str(),
                      {bounds.x + 5.0f, bounds.y + 5.0f},
                      Config::GATE_LABEL_FONT_SIZE, 1.0f,
                      Config::Colors::GATE_TEXT);
    }

    // Draw compact gate type label above body for non-IO gates
    if (type == GateType::AND || type == GateType::OR || type == GateType::XOR || type == GateType::NOT) {
        const char* label = (type == GateType::AND) ? "AND" :
                            (type == GateType::OR)  ? "OR"  :
                            (type == GateType::XOR) ? "XOR" : "NOT";
        float fontSize = Config::GATE_LABEL_FONT_SIZE * 0.9f;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), label, fontSize, 1.0f);
        Vector2 labelPos = { bounds.x + (bounds.width - textSize.x) / 2.0f,
                             bounds.y - textSize.y - 5.0f };
        VisualEffects::drawTextWithShadow(label, labelPos, fontSize,
                                          Config::Colors::GATE_TEXT, Config::Colors::GATE_SHADOW,
                                          {1.0f, 1.0f});
    }
}

void GateRenderer::renderGatePins(const LogicGate* gate) const {
    if (!gate) {
        return;
    }

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
    bool isActive = pin->getState();
    Color pinColor = isActive ? Config::Colors::PIN_STATE_ON : Config::Colors::PIN_STATE_OFF;

    // Draw a crisp ring + inner fill for clearer pins
    float outerR = pin->getClickRadius();
    float ringThickness = 2.0f;
    float innerR = std::max(outerR - ringThickness, 1.0f);

    Color ringColor = Config::Colors::GATE_OUTLINE;
    if (!isActive) {
        // Make inactive pins more contrasty on dark backgrounds
        ringColor = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.55f);
    }

    DrawRing(pos, innerR, outerR, 0.0f, 360.0f, 32, ringColor);
    Color innerColor = isActive ? pinColor : VisualEffects::lerpColor(pinColor, WHITE, 0.18f);
    DrawCircleV(pos, innerR - 1.0f, innerColor);

    if (showLabel) {
        const char* stateText = pin->getState() ? "1" : "0";
        float fontSize = Config::PIN_LABEL_FONT_SIZE;

        Vector2 labelPos;
        if (pin->getType() == PinType::INPUT_PIN) {
            labelPos = {pos.x - Config::PIN_LABEL_OFFSET, pos.y - fontSize / 2.0f};
        } else {
            labelPos = {pos.x + Config::PIN_LABEL_OFFSET / 2.0f, pos.y - fontSize / 2.0f};
        }

        if (isActive) {
            VisualEffects::drawTextWithShadow(stateText, labelPos, fontSize,
                                            Config::Colors::PIN_TEXT, Config::Colors::GATE_SHADOW,
                                            {1.0f, 1.0f});
        } else {
            DrawTextEx(GetFontDefault(), stateText, labelPos, fontSize, 1.0f, Config::Colors::PIN_TEXT);
        }
    }
}

void GateRenderer::renderAndGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // AND gate: rectangle on left, semicircle on right (with subtle shadow)
    float actualWidth = bounds.width * Config::GATE_WIDTH_RATIO;
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;

    Vector2 center = {leftX + actualWidth / 2.0f, bounds.y + bounds.height / 2.0f};
    float radius = bounds.height / 2.0f;
    float flatPartWidth = actualWidth - radius;  // Width of rectangular portion
    int segments = Config::GATE_CURVE_SEGMENTS;  // Smoothness of semicircle

    Vector2 shadowOffset = {Config::SHADOW_OFFSET, Config::SHADOW_OFFSET};
    Rectangle shadowRectPart = {
        leftX + shadowOffset.x,
        bounds.y + shadowOffset.y,
        flatPartWidth,
        bounds.height
    };
    DrawRectangleRec(shadowRectPart, Config::Colors::GATE_SHADOW);

    Rectangle rectPart = {
        leftX,
        bounds.y,
        flatPartWidth,
        bounds.height
    };
    DrawRectangleRec(rectPart, fillColor);

    // Draw semicircle using triangular segments
    Vector2 semicircleCenter = {leftX + flatPartWidth, center.y};

    // Shadow for semicircle
    Vector2 shadowCenter = {semicircleCenter.x + shadowOffset.x, semicircleCenter.y + shadowOffset.y};
    for (int i = 0; i < segments; i++) {
        float startAngle = PI/2 - (i * PI / segments);
        float endAngle = PI/2 - ((i + 1) * PI / segments);
        Vector2 sp1 = shadowCenter;
        Vector2 sp2 = { shadowCenter.x + radius * cosf(startAngle), shadowCenter.y + radius * sinf(startAngle) };
        Vector2 sp3 = { shadowCenter.x + radius * cosf(endAngle),   shadowCenter.y + radius * sinf(endAngle) };
        DrawTriangle(sp1, sp2, sp3, Config::Colors::GATE_SHADOW);
    }

    for (int i = 0; i < segments; i++) {
        // Calculate angles for current segment (from top to bottom)
        float startAngle = PI/2 - (i * PI / segments);
        float endAngle = PI/2 - ((i + 1) * PI / segments);

        Vector2 p1 = semicircleCenter;  // Center point
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

    DrawLineEx({leftX, bounds.y}, {leftX + flatPartWidth, bounds.y}, outlineThickness, outlineColor);
    DrawLineEx({leftX, bounds.y}, {leftX, bounds.y + bounds.height}, outlineThickness, outlineColor);
    DrawLineEx({leftX, bounds.y + bounds.height}, {leftX + flatPartWidth, bounds.y + bounds.height}, outlineThickness, outlineColor);

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

    // No gate label inside the shape; shown above instead
}

void GateRenderer::renderOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    float actualWidth = bounds.width * Config::GATE_OR_WIDTH_RATIO;
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
    float rightX = leftX + actualWidth;
    Vector2 rightPoint = {rightX, center.y};

    const int segments = Config::GATE_CURVE_SEGMENTS;
    float curveDepth = bounds.height * Config::GATE_CURVE_DEPTH_RATIO;
    Vector2 curvePoints[segments + 1];

    // Generate parabolic curve points for the left (back) side
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;  // 0.0 to 1.0 from top to bottom
        float y = bounds.y + t * bounds.height;

        // Parabolic curve: maximum curve at center, straight at ends
        float normalizedT = 2.0f * t - 1.0f;  // Convert to -1.0 to 1.0 range
        float curveAmount = 1.0f - normalizedT * normalizedT;  // Parabola (1 at center, 0 at ends)
        float x = leftX + curveDepth * curveAmount;

        curvePoints[i] = {x, y};
    }

    for (int i = 0; i < segments; i++) {
        DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, fillColor);
    }

    for (int i = 0; i < segments; i++) {
        DrawLineEx(curvePoints[i], curvePoints[i+1], outlineThickness, outlineColor);
    }

    DrawLineEx(curvePoints[0], rightPoint, outlineThickness, outlineColor);
    DrawLineEx(curvePoints[segments], rightPoint, outlineThickness, outlineColor);

    // Label drawn above; keep symbol clean
}

void GateRenderer::renderXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    renderOrGateSymbol(bounds, fillColor, outlineColor, outlineThickness);

    float actualWidth = bounds.width * Config::GATE_XOR_WIDTH_RATIO;
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;
    float curveOffset = bounds.height * 0.11f;
    float curveDepth = bounds.height * Config::GATE_CURVE_DEPTH_RATIO;

    const int segments = Config::GATE_CURVE_SEGMENTS;
    Vector2 prevPoint = {leftX - curveOffset, bounds.y};

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        float normalizedT = 2.0f * t - 1.0f;
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = (leftX - curveOffset) + curveDepth * curveAmount;

        Vector2 point = {x, y};
        DrawLineEx(prevPoint, point, outlineThickness, outlineColor);
        prevPoint = point;
    }

    // Label drawn above; keep symbol clean
}

void GateRenderer::renderNotGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // NOT gate: triangle with inversion bubble
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Calculate triangle dimensions for proper proportions
    float triangleHeight = bounds.height;
    float idealWidth = triangleHeight * Config::GATE_TRIANGLE_ASPECT_RATIO; // width = height * sqrt(3)/2 for equilateral triangle

    // Use configured width ratio for compact appearance
    float actualWidth = fminf(bounds.width * Config::GATE_NOT_WIDTH_RATIO, idealWidth);
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

    // Draw the inversion bubble at triangle tip
    Vector2 circleCenter = points[2]; // Right point of triangle
    float circleRadius = Config::INVERSION_BUBBLE_RADIUS; // Small but visible inversion indicator

    // Draw filled circle
    DrawCircleV(circleCenter, circleRadius, fillColor);

    // Draw circle outline with matching thickness
    DrawRing(circleCenter, circleRadius - outlineThickness/2.0f, circleRadius + outlineThickness/2.0f, 0.0f, 360.0f, 32, outlineColor);

    // Label drawn above; keep symbol clean
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

void GateRenderer::renderTriangularOrGateShape(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create triangular OR gate with curved back edge and triangular front (shape only, no text)
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Use same sizing logic as NOT gate for consistency
    float triangleHeight = bounds.height;
    float idealWidth = triangleHeight * 0.876f; // width = height * sqrt(3)/2 for equilateral

    // Use configured width ratio to prevent text clipping while maintaining good proportions
    float actualWidth = fminf(bounds.width * Config::GATE_OR_WIDTH_RATIO, idealWidth);
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;

    // Right point (triangular front)
    Vector2 rightPoint = {leftX + actualWidth, center.y};

    // Parameters for the curved back edge (similar to traditional OR gate)
    const int segments = Config::GATE_CURVE_SEGMENTS;
    float curveDepth = bounds.height * Config::GATE_XOR_CURVE_DEPTH_RATIO; // Reduced curve depth to provide more text space
    Vector2 curvePoints[segments + 1];

    // Generate parabolic curve points for the left (back) side
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        // Parabolic curve calculation - maximum curve at center, straight at ends
        float normalizedT = 2.0f * t - 1.0f; // -1 to 1
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = leftX + curveDepth * curveAmount;

        curvePoints[i] = {x, y};
    }

    // Draw shadow using curved points
    Vector2 shadowOffset = {Config::SHADOW_OFFSET, Config::SHADOW_OFFSET};
    Vector2 shadowCurvePoints[segments + 1];
    for (int i = 0; i <= segments; i++) {
        shadowCurvePoints[i] = {
            curvePoints[i].x + shadowOffset.x,
            curvePoints[i].y + shadowOffset.y
        };
    }
    Vector2 shadowRightPoint = {rightPoint.x + shadowOffset.x, rightPoint.y + shadowOffset.y};

    // Draw shadow triangles
    for (int i = 0; i < segments; i++) {
        DrawTriangle(shadowCurvePoints[i], shadowCurvePoints[i+1], shadowRightPoint, Config::Colors::GATE_SHADOW);
    }

    // Draw filled triangles to create the OR gate shape with curved back
    for (int i = 0; i < segments; i++) {
        DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, fillColor);
    }

    // Draw the curved back edge outline
    for (int i = 0; i < segments; i++) {
        DrawLineEx(curvePoints[i], curvePoints[i+1], outlineThickness, outlineColor);
    }

    // Draw the straight lines from curve endpoints to the right point (triangular front)
    DrawLineEx(curvePoints[0], rightPoint, outlineThickness, outlineColor);
    DrawLineEx(curvePoints[segments], rightPoint, outlineThickness, outlineColor);
}

void GateRenderer::renderTriangularOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Draw the OR gate shape
    renderTriangularOrGateShape(bounds, fillColor, outlineColor, outlineThickness);

    // No inner label; label rendered above in renderGateBody
}

void GateRenderer::renderTriangularXorGateShape(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Create triangular XOR gate with curved back edge and double line (shape only, no text)
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};

    // Use same sizing logic as NOT gate for consistency
    float triangleHeight = bounds.height;
    float idealWidth = triangleHeight * 0.976f; // width = height * sqrt(3)/2 for equilateral

    // Use configured width ratio to prevent text clipping while maintaining good proportions
    float actualWidth = fminf(bounds.width * Config::GATE_XOR_WIDTH_RATIO, idealWidth);
    float leftX = bounds.x + (bounds.width - actualWidth) / 2.0f;

    // Right point (triangular front)
    Vector2 rightPoint = {leftX + actualWidth, center.y};

    // Parameters for the curved back edge (similar to traditional OR gate)
    const int segments = Config::GATE_CURVE_SEGMENTS;
    float curveDepth = bounds.height * Config::GATE_XOR_CURVE_DEPTH_RATIO; // Reduced curve depth to provide more text space
    Vector2 curvePoints[segments + 1];

    // Generate parabolic curve points for the left (back) side
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        // Parabolic curve calculation - maximum curve at center, straight at ends
        float normalizedT = 2.0f * t - 1.0f; // -1 to 1
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = leftX + curveDepth * curveAmount;

        curvePoints[i] = {x, y};
    }

    // Draw shadow using curved points
    Vector2 shadowOffset = {Config::SHADOW_OFFSET, Config::SHADOW_OFFSET};
    Vector2 shadowCurvePoints[segments + 1];
    for (int i = 0; i <= segments; i++) {
        shadowCurvePoints[i] = {
            curvePoints[i].x + shadowOffset.x,
            curvePoints[i].y + shadowOffset.y
        };
    }
    Vector2 shadowRightPoint = {rightPoint.x + shadowOffset.x, rightPoint.y + shadowOffset.y};

    // Draw shadow triangles
    for (int i = 0; i < segments; i++) {
        DrawTriangle(shadowCurvePoints[i], shadowCurvePoints[i+1], shadowRightPoint, Config::Colors::GATE_SHADOW);
    }

    // Draw filled triangles to create the XOR gate shape with curved back
    for (int i = 0; i < segments; i++) {
        DrawTriangle(curvePoints[i], curvePoints[i+1], rightPoint, fillColor);
    }

    // Draw the curved back edge outline
    for (int i = 0; i < segments; i++) {
        DrawLineEx(curvePoints[i], curvePoints[i+1], outlineThickness, outlineColor);
    }

    // Draw the straight lines from curve endpoints to the right point (triangular front)
    DrawLineEx(curvePoints[0], rightPoint, outlineThickness, outlineColor);
    DrawLineEx(curvePoints[segments], rightPoint, outlineThickness, outlineColor);

    // Draw XOR double line on the left side (offset from the curved back)
    float lineOffset = Config::XOR_LINE_OFFSET;
    Vector2 secondCurvePoints[segments + 1];

    // Generate second curve points for XOR double line
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float y = bounds.y + t * bounds.height;

        // Same parabolic curve calculation but offset to the left
        float normalizedT = 2.0f * t - 1.0f; // -1 to 1
        float curveAmount = 1.0f - normalizedT * normalizedT;
        float x = (leftX - lineOffset) + curveDepth * curveAmount;

        secondCurvePoints[i] = {x, y};
    }

    // Draw the second curved line for XOR
    for (int i = 0; i < segments; i++) {
        DrawLineEx(secondCurvePoints[i], secondCurvePoints[i+1], outlineThickness, outlineColor);
    }
}

void GateRenderer::renderTriangularXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const {
    // Draw the XOR gate shape
    renderTriangularXorGateShape(bounds, fillColor, outlineColor, outlineThickness);

    // No inner label; label rendered above in renderGateBody
}

void GateRenderer::renderEnhancedGateSymbol(Rectangle bounds, Color fillColor, Color fillColorLight,
                                          Color outlineColor, float outlineThickness, float pulseValue,
                                          bool isSelected, const char* gateType) const {
    // Apply selection pulsing effect to outline color
    Color currentOutlineColor = outlineColor;
    if (isSelected) {
        currentOutlineColor = VisualEffects::lerpColor(outlineColor, Config::Colors::SELECTION_HIGHLIGHT, pulseValue);
    }

    // Draw specific gate shape based on type (no background rectangle)
    if (strcmp(gateType, "AND") == 0) {
        renderAndGateSymbol(bounds, fillColor, currentOutlineColor, outlineThickness);
    } else if (strcmp(gateType, "OR") == 0) {
        renderTriangularOrGateSymbol(bounds, fillColor, currentOutlineColor, outlineThickness);
    } else if (strcmp(gateType, "XOR") == 0) {
        renderTriangularXorGateSymbol(bounds, fillColor, currentOutlineColor, outlineThickness);
    } else if (strcmp(gateType, "NOT") == 0) {
        renderNotGateSymbol(bounds, fillColor, currentOutlineColor, outlineThickness);
    } else {
        // Fallback: rounded rectangle without inner text; label drawn above
        DrawRectangleRounded(bounds, 0.2f, 8, fillColor);
        DrawRectangleRoundedLines(bounds, 0.2f, 8, outlineThickness, currentOutlineColor);
    }

    // Note: fillColorLight reserved for future gradients
    (void)fillColorLight; // Suppress unused parameter warning
}
