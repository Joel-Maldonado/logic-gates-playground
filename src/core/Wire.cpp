#include "core/Wire.h"
#include "core/LogicGate.h"
#include <raylib.h>
#include "rlgl.h"
#include <raymath.h>
#include "ui/WireRouter.h"
#include "app/Config.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

Color Wire::Style::off = DARKGRAY;
Color Wire::Style::on = SKYBLUE;
Color Wire::Style::selected = YELLOW;

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

void Wire::update() {
    if (!sourcePin_ || !destPin_) {
        state_ = false;
        return;
    }

    try {
        bool newSourceState = sourcePin_->getState();
        if (state_ != newSourceState) {
            state_ = newSourceState;
            if (destPin_->getParentGate()) {
                 destPin_->getParentGate()->markDirty();
            }
        }
    } catch (const std::exception& e) {
        state_ = false;
    }
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
        }

        drawWirePath(controlPoints_, wireColorToUse, thickness);

        if (isSelected) {
            for (const auto& point : controlPoints_) {
                DrawCircleV(point, 4.0f, YELLOW);
            }
        }
    } catch (const std::exception& e) {
    }
}

void Wire::drawWirePath(const std::vector<Vector2>& points, Color color, float thickness) const {
    if (points.size() < 2) {
        return;
    }

    // Draw line segments between control points
    for (size_t i = 0; i < points.size() - 1; i++) {
        DrawLineEx(points[i], points[i + 1], thickness, color);
    }

    // Draw a small for the direction of signal direction if the wire is long enough
    if (points.size() >= 2) {
        Vector2 lastSegmentStart = points[points.size() - 2];
        Vector2 lastSegmentEnd = points[points.size() - 1];

        float distance = Vector2Distance(lastSegmentStart, lastSegmentEnd);

        if (distance > 30.0f) {
            // Calculate a point 80% of the way from start to end of the last segment
            float t = 0.8f;
            Vector2 arrowPos = {
                lastSegmentStart.x + t * (lastSegmentEnd.x - lastSegmentStart.x),
                lastSegmentStart.y + t * (lastSegmentEnd.y - lastSegmentStart.y)
            };

            // Calculate direction vector
            Vector2 dir = Vector2Normalize(Vector2Subtract(lastSegmentEnd, lastSegmentStart));

            // Perpendicular direction
            Vector2 perp = { -dir.y, dir.x };

            // Calculate arrow points
            Vector2 arrowTip = Vector2Add(arrowPos, Vector2Scale(dir, 8.0f));
            Vector2 arrowLeft = Vector2Subtract(arrowPos, Vector2Scale(perp, 4.0f));
            Vector2 arrowRight = Vector2Add(arrowPos, Vector2Scale(perp, 4.0f));

            // Draw arrow
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

    // Check each line segment in the wire path
    for (size_t i = 0; i < controlPoints_.size() - 1; i++) {
        Vector2 p1 = controlPoints_[i];
        Vector2 p2 = controlPoints_[i + 1];

        // Check bounding box of this line segment first for quick rejection
        Rectangle segmentBounds;
        segmentBounds.x = fmin(p1.x, p2.x) - tolerance;
        segmentBounds.y = fmin(p1.y, p2.y) - tolerance;
        segmentBounds.width = fabsf(p1.x - p2.x) + 2 * tolerance;
        segmentBounds.height = fabsf(p1.y - p2.y) + 2 * tolerance;

        if (CheckCollisionPointRec(mousePos, segmentBounds)) {
            // Check collision with the line segment
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

    // Check if mouse is over a control point (skip first and last points which  attached to pins)
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

    // Update the control point position
    controlPoints_[draggedPointIndex_] = mousePos;

    // Adjust adjacent segments to maintain orthogonal routing
    if (draggedPointIndex_ > 0 && draggedPointIndex_ < static_cast<int>(controlPoints_.size()) - 1) {
        Vector2 prevPoint = controlPoints_[draggedPointIndex_ - 1];
        // We don't need nextPoint for the calculation, just for updating

        // Determine if the previous segment is horizontal or vertical
        bool isPrevHorizontal = fabs(prevPoint.y - mousePos.y) < 0.001f;

        if (isPrevHorizontal) {
            // Previous segment is horizontal, so next segment must be vertical
            // Keep the x-coordinate of the dragged point for the next point
            controlPoints_[draggedPointIndex_ + 1].x = mousePos.x;
        } else {
            // Previous segment is vertical, so next segment must be horizontal
            // Keep the y-coordinate of the dragged point for the next point
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