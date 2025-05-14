#ifndef WIRE_ROUTER_H
#define WIRE_ROUTER_H

#include "raylib.h"
#include <vector>

/**
 * Handles wire routing logic for creating organized wire paths
 */
class WireRouter {
public:
    /**
     * Constructs a new WireRouter
     */
    WireRouter();

    /**
     * Calculates a path between two points using Manhattan-style routing
     *
     * @param startPos Starting position of the wire
     * @param endPos End position of the wire
     * @param isDestInput Whether the destination is an input pin (affects routing)
     * @param gridSize Grid size for snapping (0 for no snapping)
     * @return Vector of points representing the wire path
     */
    std::vector<Vector2> calculatePath(Vector2 startPos, Vector2 endPos, bool isDestInput = false, float gridSize = 0.0f);

    /**
     * Calculates a preview path for a wire being drawn
     *
     * @param startPos Starting position of the wire
     * @param endPos Current mouse position
     * @param isDestInput Whether the destination is an input pin (affects routing)
     * @param gridSize Grid size for snapping (0 for no snapping)
     * @return Vector of points representing the wire path
     */
    std::vector<Vector2> calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput = false, float gridSize = 0.0f);

    /**
     * Adjusts a point in a wire path
     *
     * @param path Current wire path
     * @param pointIndex Index of the point to adjust
     * @param newPosition New position for the point
     * @param gridSize Grid size for snapping (0 for no snapping)
     * @return Updated wire path
     */
    std::vector<Vector2> adjustPathPoint(const std::vector<Vector2>& path, int pointIndex, Vector2 newPosition, float gridSize = 0.0f);

    /**
     * Snaps a point to the grid
     *
     * @param point Point to snap
     * @param gridSize Grid size
     * @return Snapped point
     */
    Vector2 snapToGrid(Vector2 point, float gridSize);

private:
    /**
     * Determines the best routing direction based on the relative positions
     *
     * @param startPos Starting position
     * @param endPos End position
     * @return True if horizontal first, false if vertical first
     */
    bool shouldRouteHorizontalFirst(Vector2 startPos, Vector2 endPos);

    /**
     * Simplifies a path by removing unnecessary points
     *
     * @param path Path to simplify
     * @return Simplified path
     */
    std::vector<Vector2> simplifyPath(const std::vector<Vector2>& path);
};

#endif // WIRE_ROUTER_H
