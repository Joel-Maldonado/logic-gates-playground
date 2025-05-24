#include "ui/WireRouter.h"
#include <raymath.h>
#include "app/Config.h"
#include <cmath>
#include <algorithm>

WireRouter::WireRouter() {
}

std::vector<Vector2> WireRouter::calculatePath(Vector2 startPos, Vector2 endPos, bool isDestInput) {
    std::vector<Vector2> path;
    path.push_back(startPos);

    // Direct line if already aligned horizontally or vertically
    if (fabs(startPos.x - endPos.x) < 0.001f || fabs(startPos.y - endPos.y) < 0.001f) {
        path.push_back(endPos);
        return path;
    }

    // Add small horizontal offset from output pin for cleaner routing
    Vector2 initialHorizontalPoint = { startPos.x + Config::WIRE_HORIZONTAL_OFFSET, startPos.y };
    path.push_back(initialHorizontalPoint);

    if (isDestInput) {
        // For input destinations: go right, then down/up, then left to input
        Vector2 verticalAlignPoint = { initialHorizontalPoint.x, endPos.y };
        path.push_back(verticalAlignPoint);
        path.push_back(endPos);
    } else {
        // For output destinations: choose optimal routing direction
        bool horizontalFirst = shouldRouteHorizontalFirst(initialHorizontalPoint, endPos);

        if (horizontalFirst) {
            // Go horizontal first, then vertical
            Vector2 midPoint = { endPos.x, initialHorizontalPoint.y };
            path.push_back(midPoint);
        } else {
            // Go vertical first, then horizontal
            Vector2 midPoint = { initialHorizontalPoint.x, endPos.y };
            path.push_back(midPoint);
        }

        path.push_back(endPos);
    }

    return path;
}

std::vector<Vector2> WireRouter::calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput) {
    return calculatePath(startPos, endPos, isDestInput);
}

std::vector<Vector2> WireRouter::adjustPathPoint(const std::vector<Vector2>& path, int pointIndex, Vector2 newPosition) {
    if (path.empty() || pointIndex < 0 || pointIndex >= static_cast<int>(path.size())) {
        return path;
    }

    std::vector<Vector2> newPath = path;
    newPath[pointIndex] = newPosition;

    // Adjust adjacent points to maintain orthogonal wire segments
    if (pointIndex > 0 && pointIndex < static_cast<int>(path.size()) - 1) {
        Vector2 prevPoint = newPath[pointIndex - 1];
        bool isHorizontalSegment = fabs(prevPoint.y - newPosition.y) < 0.001f;

        if (isHorizontalSegment) {
            // Maintain horizontal alignment with previous point
            newPath[pointIndex - 1].y = newPosition.y;
            newPath[pointIndex + 1].x = newPosition.x;
        } else {
            // Maintain vertical alignment with previous point
            newPath[pointIndex - 1].x = newPosition.x;
            newPath[pointIndex + 1].y = newPosition.y;
        }
    }

    return simplifyPath(newPath);
}

bool WireRouter::shouldRouteHorizontalFirst(Vector2 startPos, Vector2 endPos) {
    // Choose routing direction based on which distance is greater
    float dx = fabs(endPos.x - startPos.x);
    float dy = fabs(endPos.y - startPos.y);
    return dx > dy;  // Prefer horizontal routing for wider gaps
}

std::vector<Vector2> WireRouter::simplifyPath(const std::vector<Vector2>& path) {
    if (path.size() <= 2) {
        return path;
    }

    std::vector<Vector2> simplified;
    simplified.push_back(path[0]);

    // Remove redundant points that lie on straight lines
    for (size_t i = 1; i < path.size() - 1; i++) {
        Vector2 prev = path[i - 1];
        Vector2 current = path[i];
        Vector2 next = path[i + 1];

        // Check if three consecutive points are collinear (on same line)
        bool isCollinear =
            (fabs(prev.x - current.x) < 0.001f && fabs(current.x - next.x) < 0.001f) ||  // Vertical line
            (fabs(prev.y - current.y) < 0.001f && fabs(current.y - next.y) < 0.001f);   // Horizontal line

        if (!isCollinear) {
            simplified.push_back(current);  // Keep corner points only
        }
    }

    simplified.push_back(path.back());
    return simplified;
}
