#include "ui/WireRouter.h"
#include "raymath.h"
#include "app/Config.h"
#include <cmath>
#include <algorithm>

WireRouter::WireRouter() {
}

std::vector<Vector2> WireRouter::calculatePath(Vector2 startPos, Vector2 endPos, bool isDestInput) {
    std::vector<Vector2> path;
    path.push_back(startPos);

    // Use direct straight line for aligned points
    if (fabs(startPos.x - endPos.x) < 0.001f || fabs(startPos.y - endPos.y) < 0.001f) {
        path.push_back(endPos);
        return path;
    }

    // Start with small horizontal segment from output pin to prevent visual clipping
    const float horizontalOffset = 10.0f;
    Vector2 initialHorizontalPoint = { startPos.x + horizontalOffset, startPos.y };
    path.push_back(initialHorizontalPoint);

    // For input pins: ensure horizontal approach with vertical segment first
    if (isDestInput) {
        // First align vertically with the end point
        Vector2 verticalAlignPoint = { initialHorizontalPoint.x, endPos.y };
        path.push_back(verticalAlignPoint);

        // Then connect horizontally to the input pin
        path.push_back(endPos);
    } else {
        // For other destinations: choose optimal routing based on distance
        bool horizontalFirst = shouldRouteHorizontalFirst(initialHorizontalPoint, endPos);

        if (horizontalFirst) {
            // Route horizontally then vertically
            Vector2 midPoint = { endPos.x, initialHorizontalPoint.y };
            path.push_back(midPoint);
        } else {
            // Route vertically then horizontally
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

    // For intermediate points, maintain orthogonal segments by adjusting adjacent points
    if (pointIndex > 0 && pointIndex < static_cast<int>(path.size()) - 1) {
        Vector2 prevPoint = newPath[pointIndex - 1];
        bool isHorizontalSegment = fabs(prevPoint.y - newPosition.y) < 0.001f;

        if (isHorizontalSegment) {
            // For horizontal segments: align y-coordinates and propagate x-coordinate
            newPath[pointIndex - 1].y = newPosition.y;
            newPath[pointIndex + 1].x = newPosition.x;
        } else {
            // For vertical segments: align x-coordinates and propagate y-coordinate
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
    return dx > dy;
}

std::vector<Vector2> WireRouter::simplifyPath(const std::vector<Vector2>& path) {
    if (path.size() <= 2) {
        return path;
    }

    std::vector<Vector2> simplified;
    simplified.push_back(path[0]);

    // Remove redundant points that form straight lines
    for (size_t i = 1; i < path.size() - 1; i++) {
        Vector2 prev = path[i - 1];
        Vector2 current = path[i];
        Vector2 next = path[i + 1];

        // Check if three points are collinear (form a straight line)
        bool isCollinear =
            (fabs(prev.x - current.x) < 0.001f && fabs(current.x - next.x) < 0.001f) ||
            (fabs(prev.y - current.y) < 0.001f && fabs(current.y - next.y) < 0.001f);

        if (!isCollinear) {
            simplified.push_back(current);
        }
    }

    simplified.push_back(path.back());
    return simplified;
}
