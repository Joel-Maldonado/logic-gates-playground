#include "ui/PaletteManager.h"

#include "ui/GateFactory.h"
#include "ui/GateGeometry.h"

#include <algorithm>

namespace {

Color mix(Color a, Color b, float t) {
    const float k = std::clamp(t, 0.0f, 1.0f);
    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * k),
        static_cast<unsigned char>(a.g + (b.g - a.g) * k),
        static_cast<unsigned char>(a.b + (b.b - a.b) * k),
        static_cast<unsigned char>(a.a + (b.a - a.a) * k)
    };
}

Color gateAccent(GateType type, const DesignTokens& tokens) {
    switch (type) {
        case GateType::AND: return tokens.colors.gateAccentAnd;
        case GateType::OR: return tokens.colors.gateAccentOr;
        case GateType::XOR: return tokens.colors.gateAccentXor;
        case GateType::NOT: return tokens.colors.gateAccentNot;
        case GateType::INPUT_SOURCE:
        case GateType::OUTPUT_SINK:
            return tokens.colors.accentPrimary;
        case GateType::NONE:
            break;
    }

    return tokens.colors.accentPrimary;
}

void drawOpenStroke(const std::vector<Vector2>& points, float thickness, Color color) {
    if (points.size() < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < points.size(); ++i) {
        DrawLineEx(points[i], points[i + 1], thickness, color);
    }
}

} // namespace

PaletteManager::PaletteManager(std::shared_ptr<CircuitSimulator> simulator)
    : simulator_(std::move(simulator)),
      bounds_({0.0f, 0.0f, 220.0f, 400.0f}),
      selectedGateType_(GateType::NONE),
      isDraggingGate_(false),
      draggedGateType_(GateType::NONE),
      currentDragPos_({0.0f, 0.0f}) {
}

void PaletteManager::initialize() {
    gatePalette_.clear();

    const GateType types[] = {
        GateType::INPUT_SOURCE,
        GateType::OUTPUT_SINK,
        GateType::AND,
        GateType::OR,
        GateType::XOR,
        GateType::NOT
    };

    float y = bounds_.y + 54.0f;
    for (GateType type : types) {
        PaletteItem item{};
        item.bounds = {bounds_.x + 10.0f, y, bounds_.width - 20.0f, 48.0f};
        item.type = type;
        item.label = getGateTypeName(type);
        gatePalette_.push_back(item);
        y += 56.0f;
    }
}

void PaletteManager::setBounds(Rectangle bounds) {
    bounds_ = bounds;
    initialize();
}

Rectangle PaletteManager::getPaletteBounds() const {
    return bounds_;
}

void PaletteManager::render(const DesignTokens& tokens, Rectangle canvasBounds) const {
    DrawRectangleRec(bounds_, tokens.colors.panelBackground);
    DrawLineEx({bounds_.x + bounds_.width, bounds_.y},
               {bounds_.x + bounds_.width, bounds_.y + bounds_.height},
               1.0f,
               tokens.colors.panelBorder);

    DrawTextEx(tokens.typography.ui,
               "Components",
               {bounds_.x + 12.0f, bounds_.y + 14.0f},
               tokens.typography.bodySize,
               1.0f,
               tokens.colors.textPrimary);

    Vector2 mouse = GetMousePosition();
    for (const PaletteItem& item : gatePalette_) {
        const bool hovered = CheckCollisionPointRec(mouse, item.bounds);
        const bool selected = selectedGateType_ == item.type;

        Color rowColor = tokens.colors.panelElevated;
        if (selected) {
            rowColor = Fade(tokens.colors.accentSelection, 0.2f);
        } else if (hovered) {
            rowColor = Fade(tokens.colors.accentPrimary, 0.14f);
        }

        DrawRectangleRounded(item.bounds, 0.18f, 8, rowColor);
        DrawRectangleRoundedLines(item.bounds, 0.18f, 8, 1.0f,
                                  selected ? tokens.colors.accentSelection : tokens.colors.panelBorder);

        Rectangle iconBounds = {item.bounds.x + 8.0f, item.bounds.y + 7.0f, 34.0f, 34.0f};
        drawGateIcon(item.type, iconBounds, tokens);

        DrawTextEx(tokens.typography.ui,
                   item.label.c_str(),
                   {item.bounds.x + 50.0f, item.bounds.y + 14.0f},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
    }

    if (isDraggingGate_ && draggedGateType_ != GateType::NONE) {
        const bool overCanvas = CheckCollisionPointRec(currentDragPos_, canvasBounds);
        if (!overCanvas) {
            Rectangle iconBounds = {
                currentDragPos_.x - 16.0f,
                currentDragPos_.y - 16.0f,
                32.0f,
                32.0f
            };
            drawGateIcon(draggedGateType_, iconBounds, tokens);
            DrawCircleLines(static_cast<int>(currentDragPos_.x),
                            static_cast<int>(currentDragPos_.y),
                            18.0f,
                            Fade(tokens.colors.ghostStroke, 0.8f));
        }
    }

    DrawTextEx(tokens.typography.mono,
               "Drag to place",
               {bounds_.x + 12.0f, bounds_.y + bounds_.height - 46.0f},
               tokens.typography.smallSize,
               1.0f,
               tokens.colors.textMuted);
    DrawTextEx(tokens.typography.mono,
               "Ctrl+K commands",
               {bounds_.x + 12.0f, bounds_.y + bounds_.height - 26.0f},
               tokens.typography.smallSize,
               1.0f,
               tokens.colors.textMuted);
}

bool PaletteManager::handleClick(Vector2 mousePos) {
    if (!CheckCollisionPointRec(mousePos, bounds_)) {
        return false;
    }

    for (const PaletteItem& item : gatePalette_) {
        if (CheckCollisionPointRec(mousePos, item.bounds)) {
            selectedGateType_ = item.type;
            return true;
        }
    }

    return true;
}

bool PaletteManager::startDraggingGate(Vector2 mousePos) {
    if (!CheckCollisionPointRec(mousePos, bounds_)) {
        return false;
    }

    for (const PaletteItem& item : gatePalette_) {
        if (CheckCollisionPointRec(mousePos, item.bounds)) {
            isDraggingGate_ = true;
            draggedGateType_ = item.type;
            selectedGateType_ = item.type;
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

    std::unique_ptr<LogicGate> gate = createGateInstance(draggedGateType_, worldPos);
    isDraggingGate_ = false;
    GateType dragged = draggedGateType_;
    draggedGateType_ = GateType::NONE;

    if (!gate || !simulator_) {
        return nullptr;
    }

    LogicGate* raw = simulator_->addGate(std::move(gate));
    selectedGateType_ = dragged;
    return raw;
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

std::unique_ptr<LogicGate> PaletteManager::createSelectedGateInstance(Vector2 position) {
    return createGateInstance(selectedGateType_, position);
}

std::unique_ptr<LogicGate> PaletteManager::createGateInstance(GateType type, Vector2 position) {
    if (!simulator_ || type == GateType::NONE) {
        return nullptr;
    }

    const std::string gateId = "gate" + std::to_string(simulator_->useNextGateId());
    return GateFactory::createGate(toGateKind(type), gateId, position);
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
        case GateType::OUTPUT_SINK: return "Output";
        case GateType::AND: return "AND";
        case GateType::OR: return "OR";
        case GateType::XOR: return "XOR";
        case GateType::NOT: return "NOT";
        default: return "Unknown";
    }
}

GateKind PaletteManager::toGateKind(GateType type) {
    switch (type) {
        case GateType::INPUT_SOURCE: return GateKind::INPUT_SOURCE;
        case GateType::OUTPUT_SINK: return GateKind::OUTPUT_SINK;
        case GateType::AND: return GateKind::AND_GATE;
        case GateType::OR: return GateKind::OR_GATE;
        case GateType::XOR: return GateKind::XOR_GATE;
        case GateType::NOT: return GateKind::NOT_GATE;
        default: return GateKind::AND_GATE;
    }
}

void PaletteManager::handleWindowResize() {
    initialize();
}

void PaletteManager::drawGateIcon(GateType type, Rectangle iconBounds, const DesignTokens& tokens) const {
    const GateKind kind = toGateKind(type);
    const GateShapeData shape = GateGeometry::buildShape(kind, iconBounds);

    const Color accent = gateAccent(type, tokens);
    const Color fill = Fade(mix(tokens.colors.gateFill, accent, 0.22f), 0.95f);
    const Color stroke = tokens.colors.gateStroke;

    if (shape.circular) {
        DrawCircleV(shape.circleCenter, shape.circleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.circleCenter.x),
                        static_cast<int>(shape.circleCenter.y),
                        shape.circleRadius,
                        stroke);
        return;
    }

    if (shape.fillPath.size() >= 3) {
        Vector2 center{0.0f, 0.0f};
        for (const Vector2& p : shape.fillPath) {
            center.x += p.x;
            center.y += p.y;
        }
        center.x /= static_cast<float>(shape.fillPath.size());
        center.y /= static_cast<float>(shape.fillPath.size());

        for (size_t i = 0; i < shape.fillPath.size(); ++i) {
            const Vector2 a = shape.fillPath[i];
            const Vector2 b = shape.fillPath[(i + 1) % shape.fillPath.size()];
            DrawTriangle(center, a, b, fill);
            DrawLineEx(a, b, 1.0f, stroke);
        }
    }

    for (const auto& accentStroke : shape.accentStrokes) {
        drawOpenStroke(accentStroke, 1.0f, mix(stroke, accent, 0.6f));
    }

    if (shape.hasBubble) {
        DrawCircleV(shape.bubbleCenter, shape.bubbleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.bubbleCenter.x),
                        static_cast<int>(shape.bubbleCenter.y),
                        shape.bubbleRadius,
                        stroke);
    }
}
