#include "core/Wire.h"
#include "core/LogicGate.h"
#include "ui/WireRouter.h"
#include <raylib.h>
#include <raymath.h>
#include <stdexcept>
#include <cmath>

Wire::Wire(GatePin* srcPin, GatePin* dstPin)
    : state_(false),
      sourcePin_(srcPin),
      destPin_(dstPin),
      isDraggingPoint_(false),
      draggedPointIndex_(-1),
      draggedPrevSegmentHorizontal_(false) {
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

    bool newSourceState = sourcePin_->getState();
    if (state_ != newSourceState) {
        state_ = newSourceState;
        if (destPin_->getParentGate()) {
            destPin_->getParentGate()->markDirty();
        }
        return true;
    }

    return false;
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

    const Vector2 startPos = sourcePin_->getAbsolutePosition();
    const Vector2 endPos = destPin_->getAbsolutePosition();

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
            const Vector2 prevPoint = controlPoints_[i - 1];
            const Vector2 current = controlPoints_[i];
            draggedPrevSegmentHorizontal_ = fabsf(prevPoint.y - current.y) < 0.001f;
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

    if (draggedPointIndex_ <= 0 || draggedPointIndex_ >= static_cast<int>(controlPoints_.size()) - 1) {
        return;
    }

    const Vector2 prevPoint = controlPoints_[draggedPointIndex_ - 1];
    Vector2 constrainedPoint = controlPoints_[draggedPointIndex_];
    if (draggedPrevSegmentHorizontal_) {
        constrainedPoint.x = mousePos.x;
        constrainedPoint.y = prevPoint.y;
        controlPoints_[draggedPointIndex_ + 1].x = constrainedPoint.x;
    } else {
        constrainedPoint.x = prevPoint.x;
        constrainedPoint.y = mousePos.y;
        controlPoints_[draggedPointIndex_ + 1].y = constrainedPoint.y;
    }

    controlPoints_[draggedPointIndex_] = constrainedPoint;
}

void Wire::stopDraggingPoint() {
    isDraggingPoint_ = false;
    draggedPointIndex_ = -1;
    draggedPrevSegmentHorizontal_ = false;
}

bool Wire::isDraggingPoint() const {
    return isDraggingPoint_;
}

bool Wire::isMouseOver(Vector2 mousePos, float tolerance) const {
    if (!sourcePin_ || !destPin_ || controlPoints_.size() < 2) {
        return false;
    }

    for (size_t i = 0; i < controlPoints_.size() - 1; i++) {
        const Vector2 p1 = controlPoints_[i];
        const Vector2 p2 = controlPoints_[i + 1];

        Rectangle segmentBounds;
        segmentBounds.x = fminf(p1.x, p2.x) - tolerance;
        segmentBounds.y = fminf(p1.y, p2.y) - tolerance;
        segmentBounds.width = fabsf(p1.x - p2.x) + 2 * tolerance;
        segmentBounds.height = fabsf(p1.y - p2.y) + 2 * tolerance;

        if (CheckCollisionPointRec(mousePos, segmentBounds) &&
            CheckCollisionPointLine(mousePos, p1, p2, static_cast<int>(ceilf(tolerance)))) {
            return true;
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
