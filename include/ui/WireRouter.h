#ifndef WIRE_ROUTER_H
#define WIRE_ROUTER_H

#include <raylib.h>
#include <vector>

/**
 * Handles wire routing logic for creating organized wire paths
 */
class WireRouter {
public:
    WireRouter();
    std::vector<Vector2> calculatePath(Vector2 startPos, Vector2 endPos, bool isDestInput = false);
    std::vector<Vector2> calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput = false);
    std::vector<Vector2> adjustPathPoint(const std::vector<Vector2>& path, int pointIndex, Vector2 newPosition);

private:
    bool shouldRouteHorizontalFirst(Vector2 startPos, Vector2 endPos);
    std::vector<Vector2> simplifyPath(const std::vector<Vector2>& path);
};

#endif // WIRE_ROUTER_H
