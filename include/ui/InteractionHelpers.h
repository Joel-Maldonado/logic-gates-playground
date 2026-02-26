#ifndef INTERACTION_HELPERS_H
#define INTERACTION_HELPERS_H

#include "app/Config.h"
#include <raylib.h>
#include <cmath>

namespace InteractionHelpers {

enum class DragAxis {
    NONE,
    HORIZONTAL,
    VERTICAL
};

inline float distanceSquared(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

inline bool exceedsDragThreshold(Vector2 start, Vector2 current, float threshold = Config::DRAG_THRESHOLD) {
    return distanceSquared(start, current) > threshold * threshold;
}

inline bool isClickWithinThreshold(Vector2 start, Vector2 current, float threshold = Config::DRAG_THRESHOLD) {
    return !exceedsDragThreshold(start, current, threshold);
}

inline DragAxis determineDominantAxis(Vector2 start, Vector2 current) {
    float dx = fabsf(current.x - start.x);
    float dy = fabsf(current.y - start.y);

    if (dx == 0.0f && dy == 0.0f) {
        return DragAxis::NONE;
    }

    return (dx >= dy) ? DragAxis::HORIZONTAL : DragAxis::VERTICAL;
}

inline Vector2 applyAxisLock(Vector2 position, Vector2 anchor, DragAxis axis) {
    if (axis == DragAxis::HORIZONTAL) {
        position.y = anchor.y;
    } else if (axis == DragAxis::VERTICAL) {
        position.x = anchor.x;
    }

    return position;
}

} // namespace InteractionHelpers

#endif // INTERACTION_HELPERS_H
