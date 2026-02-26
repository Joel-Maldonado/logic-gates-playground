#include "core/Wire.h"
#include "core/LogicGate.h"
#include <raylib.h>
#include "rlgl.h"
#include <raymath.h>
#include "ui/WireRouter.h"
#include "ui/VisualEffects.h"
#include "app/Config.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

Color Wire::Style::off = Config::Colors::WIRE_OFF;
Color Wire::Style::on = Config::Colors::WIRE_ON;
Color Wire::Style::selected = Config::Colors::WIRE_SELECTED;

Wire::Wire(GatePin* srcPin, GatePin* dstPin)
    : isSelected(false), state_(false), sourcePin_(srcPin), destPin_(dstPin),
      isDraggingPoint_(false), draggedPointIndex_(-1) {
    if (!sourcePin_) {
        throw std::invalid_argument("Source pin cannot be null");
    }
    if (!destPin_) {
        throw std::invalid_argument("Destination pin cannot be null");
    }
    if (sourcePin_->getType() != PinType::OUTPUT_PIN) {
        throw std::invalid_argument("Wire source must be an OUTPUT_PIN");
    }
    if (destPin_->getType() != PinType::INPUT_PIN) {
        throw std::invalid_argument("Wire destination must be an INPUT_PIN");
    }
    if (destPin_->isConnectedInput()) {
        throw std::runtime_error("Destination input pin is already connected");
    }

    destPin_->connectTo(sourcePin_);

    LogicGate* srcParentGate = sourcePin_->getParentGate();
    LogicGate* dstParentGate = destPin_->getParentGate();

    if (srcParentGate) {
        srcParentGate->addWire(this);
    }
    if (dstParentGate && srcParentGate != dstParentGate) {
        dstParentGate->addWire(this);
    }

    recalculatePath();
    update();
}

bool Wire::update() {
    if (!sourcePin_ || !destPin_) {
        state_ = false;
        return false;
    }

    try {
        bool newSourceState = sourcePin_->getState();
        if (state_ != newSourceState) {
            state_ = newSourceState;
            if (destPin_->getParentGate()) {
                 destPin_->getParentGate()->markDirty();
            }
            return true;
        }
    } catch (const std::exception& e) {
        state_ = false;
        TraceLog(LOG_WARNING, "Wire update failed: %s", e.what());
    }

    return false;
}

void Wire::draw() const {
    if (!sourcePin_ || !destPin_ || controlPoints_.empty()) {
        return;
    }

    try {
        Color wireColorToUse = state_ ? Style::on : Style::off;
        float thickness = Config::WIRE_THICKNESS_NORMAL;

        if (isSelected) {
            wireColorToUse = Style::selected;
            thickness = Config::WIRE_THICKNESS_SELECTED;
        } else if (isHovered_) {
            // Subtle hover accent without overwhelming the scene
            wireColorToUse = VisualEffects::lerpColor(wireColorToUse, Config::Colors::WIRE_HOVER, 0.35f);
            thickness += 0.5f;
        }

        drawEnhancedWirePath(controlPoints_, wireColorToUse, thickness);

        // Draw animated signal dot for active wires
        if (state_ && !isSelected) {
            float signalProgress = fmod(GetTime() * 2.0f, 1.0f);  // 2-second cycle
            if (!controlPoints_.empty()) {
                // Calculate total wire length and individual segment lengths
                float totalLength = 0.0f;
                std::vector<float> segmentLengths;
                for (size_t i = 0; i < controlPoints_.size() - 1; i++) {
                    float length = Vector2Distance(controlPoints_[i], controlPoints_[i + 1]);
                    segmentLengths.push_back(length);
                    totalLength += length;
                }

                if (totalLength > 0.0f) {
                    // Find position along wire path based on animation progress
                    float targetDistance = signalProgress * totalLength;
                    float currentDistance = 0.0f;

                    for (size_t i = 0; i < segmentLengths.size(); i++) {
                        if (currentDistance + segmentLengths[i] >= targetDistance) {
                            // Interpolate position within current segment
                            float t = (targetDistance - currentDistance) / segmentLengths[i];
                            Vector2 position = Vector2Lerp(controlPoints_[i], controlPoints_[i + 1], t);
                            VisualEffects::drawCircleWithGlow(position, 3.0f, Config::Colors::WIRE_ON,
                                                             Fade(Config::Colors::WIRE_ON, 0.35f), 6.0f);
                            break;
                        }
                        currentDistance += segmentLengths[i];
                    }
                }
            }
        }

        if (isSelected) {
            for (const auto& point : controlPoints_) {
                DrawCircleV(point, 4.0f, Config::Colors::WIRE_SELECTED);
            }
        }
    } catch (const std::exception& e) {
        TraceLog(LOG_WARNING, "Wire draw failed: %s", e.what());
    }
}

void Wire::drawWirePath(const std::vector<Vector2>& points, Color color, float thickness) const {
    if (points.size() < 2) {
        return;
    }

    for (size_t i = 0; i < points.size() - 1; i++) {
        DrawLineEx(points[i], points[i + 1], thickness, color);
    }

    if (points.size() >= 2) {
        Vector2 lastSegmentStart = points[points.size() - 2];
        Vector2 lastSegmentEnd = points[points.size() - 1];

        float distance = Vector2Distance(lastSegmentStart, lastSegmentEnd);

        if (distance > 30.0f) {
            float t = 0.8f;
            Vector2 arrowPos = {
                lastSegmentStart.x + t * (lastSegmentEnd.x - lastSegmentStart.x),
                lastSegmentStart.y + t * (lastSegmentEnd.y - lastSegmentStart.y)
            };

            Vector2 dir = Vector2Normalize(Vector2Subtract(lastSegmentEnd, lastSegmentStart));
            Vector2 perp = { -dir.y, dir.x };

            Vector2 arrowTip = Vector2Add(arrowPos, Vector2Scale(dir, 8.0f));
            Vector2 arrowLeft = Vector2Subtract(arrowPos, Vector2Scale(perp, 4.0f));
            Vector2 arrowRight = Vector2Add(arrowPos, Vector2Scale(perp, 4.0f));

            DrawTriangle(arrowTip, arrowLeft, arrowRight, color);
        }
    }
}

void Wire::drawEnhancedWirePath(const std::vector<Vector2>& points, Color color, float thickness) const {
    if (points.size() < 2) {
        return;
    }

    // Draw segments with optional glow for active/selected/hovered wires
    bool glow = state_ || isSelected || isHovered_;
    for (size_t i = 0; i < points.size() - 1; i++) {
        if (glow) {
            Color glowColor = Fade(color, 0.35f);
            VisualEffects::drawLineWithGlow(points[i], points[i + 1], thickness, color, glowColor, thickness * 1.75f);
        } else {
            DrawLineEx(points[i], points[i + 1], thickness, color);
        }
    }

    // Soften elbows by drawing small round joints
    if (points.size() > 2) {
        float jointRadius = std::max(thickness * 0.6f, 1.0f);
        for (size_t i = 1; i < points.size() - 1; i++) {
            DrawCircleV(points[i], jointRadius, color);
        }
    }

    if (points.size() >= 2) {
        Vector2 lastSegmentStart = points[points.size() - 2];
        Vector2 lastSegmentEnd = points[points.size() - 1];
        float distance = Vector2Distance(lastSegmentStart, lastSegmentEnd);

        if (distance > 30.0f) {
            // Only draw direction indicator when the wire is active or selected to reduce visual noise
            bool showArrow = (state_ && !isSelected) || isSelected;
            if (!showArrow) return;

            float t = isSelected ? 0.6f : 0.5f;  // Slightly offset when selected

            Vector2 arrowPos = {
                lastSegmentStart.x + t * (lastSegmentEnd.x - lastSegmentStart.x),
                lastSegmentStart.y + t * (lastSegmentEnd.y - lastSegmentStart.y)
            };

            Vector2 dir = Vector2Normalize(Vector2Subtract(lastSegmentEnd, lastSegmentStart));
            Vector2 perp = { -dir.y, dir.x };

            float arrowSize = isSelected ? 9.0f : 8.0f;
            Vector2 arrowTip = Vector2Add(arrowPos, Vector2Scale(dir, arrowSize));
            Vector2 arrowLeft = Vector2Subtract(arrowPos, Vector2Scale(perp, arrowSize * 0.5f));
            Vector2 arrowRight = Vector2Add(arrowPos, Vector2Scale(perp, arrowSize * 0.5f));

            DrawTriangle(arrowTip, arrowLeft, arrowRight, color);
        }
    }
}

void Wire::setSelected(bool selected) {
    isSelected = selected;
}

bool Wire::getIsSelected() const {
    return isSelected;
}

bool Wire::isMouseOver(Vector2 mousePos, float tolerance) const {
    if (!sourcePin_ || !destPin_ || controlPoints_.size() < 2) {
        return false;
    }

    for (size_t i = 0; i < controlPoints_.size() - 1; i++) {
        Vector2 p1 = controlPoints_[i];
        Vector2 p2 = controlPoints_[i + 1];

        Rectangle segmentBounds;
        segmentBounds.x = fmin(p1.x, p2.x) - tolerance;
        segmentBounds.y = fmin(p1.y, p2.y) - tolerance;
        segmentBounds.width = fabsf(p1.x - p2.x) + 2 * tolerance;
        segmentBounds.height = fabsf(p1.y - p2.y) + 2 * tolerance;

        if (CheckCollisionPointRec(mousePos, segmentBounds)) {
            if (CheckCollisionPointLine(mousePos, p1, p2, static_cast<int>(ceil(tolerance)))) {
                return true;
            }
        }
    }

    return false;
}

GatePin* Wire::getSourcePin() const {
    return sourcePin_;
}

GatePin* Wire::getDestPin() const {
    return destPin_;
}

bool Wire::getState() const {
    return state_;
}

void Wire::setControlPoints(const std::vector<Vector2>& points) {
    controlPoints_ = points;
}

const std::vector<Vector2>& Wire::getControlPoints() const {
    return controlPoints_;
}

void Wire::recalculatePath() {
    if (!sourcePin_ || !destPin_) {
        return;
    }

    Vector2 startPos = sourcePin_->getAbsolutePosition();
    Vector2 endPos = destPin_->getAbsolutePosition();

    WireRouter router;
    controlPoints_ = router.calculatePath(startPos, endPos, true);
}

bool Wire::startDraggingPoint(Vector2 mousePos, float tolerance) {
    if (controlPoints_.size() < 3) {
        return false;
    }

    for (size_t i = 1; i < controlPoints_.size() - 1; i++) {
        if (Vector2Distance(mousePos, controlPoints_[i]) <= tolerance) {
            isDraggingPoint_ = true;
            draggedPointIndex_ = static_cast<int>(i);
            return true;
        }
    }

    return false;
}

void Wire::updateDraggedPoint(Vector2 mousePos) {
    if (!isDraggingPoint_ || draggedPointIndex_ < 0 ||
        draggedPointIndex_ >= static_cast<int>(controlPoints_.size())) {
        return;
    }

    controlPoints_[draggedPointIndex_] = mousePos;

    if (draggedPointIndex_ > 0 && draggedPointIndex_ < static_cast<int>(controlPoints_.size()) - 1) {
        Vector2 prevPoint = controlPoints_[draggedPointIndex_ - 1];
        bool isPrevHorizontal = fabs(prevPoint.y - mousePos.y) < 0.001f;

        if (isPrevHorizontal) {
            controlPoints_[draggedPointIndex_ + 1].x = mousePos.x;
        } else {
            controlPoints_[draggedPointIndex_ + 1].y = mousePos.y;
        }
    }
}

void Wire::stopDraggingPoint() {
    isDraggingPoint_ = false;
    draggedPointIndex_ = -1;
}

bool Wire::isDraggingPoint() const {
    return isDraggingPoint_;
}

void Wire::onGateDeleted(LogicGate* deletedGate) {
    bool changed = false;
    if (sourcePin_ && sourcePin_->getParentGate() == deletedGate) {
        sourcePin_ = nullptr;
        changed = true;
    }
    if (destPin_ && destPin_->getParentGate() == deletedGate) {
        destPin_ = nullptr;
        changed = true;
    }

    if (changed) {
        this->state_ = false;
    }
}
