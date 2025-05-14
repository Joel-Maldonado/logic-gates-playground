#ifndef WIRE_H
#define WIRE_H

#include "core/LogicGate.h" // For LogicGate, GatePin, Vector2
#include "raylib.h"    // For Color, Vector2, drawing functions
#include <vector>

class Wire {
public:
    bool isSelected;

private:
    bool state_; // Current logical state of the wire
    GatePin* sourcePin_;
    GatePin* destPin_;
    std::vector<Vector2> controlPoints_; // Control points for the wire path
    bool isDraggingPoint_; // Whether a control point is being dragged
    int draggedPointIndex_; // Index of the control point being dragged

public:
    struct Style {
        static Color off;
        static Color on;
        static Color selected;
    };

    /**
     * Constructs a Wire connecting an output pin of a source gate to an input pin of a destination gate.
     * @param srcPin Pointer to the source GatePin (must be an OUTPUT_PIN).
     * @param dstPin Pointer to the destination GatePin (must be an INPUT_PIN).
     * @throws std::invalid_argument if srcPin or dstPin is nullptr, or if pins are of incorrect types or already connected.
     */
    Wire(GatePin* srcPin, GatePin* dstPin); // Changed constructor to take GatePin pointers

    /**
     * Updates the wire's logical state based on its source gate's output pin.
     * Also propagates this state to its destination gate's input pin.
     * This should be called regularly in the simulation loop.
     */
    void update();

    /**
     * Draws the wire on the screen using Raylib.
     * The wire's color depends on its current logical state and selection state.
     * The wire is drawn as a series of connected line segments following the control points.
     */
    void draw() const;

    /**
     * Sets the control points for the wire path
     * @param points Vector of control points
     */
    void setControlPoints(const std::vector<Vector2>& points);

    /**
     * Gets the control points for the wire path
     * @return Vector of control points
     */
    const std::vector<Vector2>& getControlPoints() const;

    /**
     * Recalculates the wire path based on the current positions of the source and destination pins
     */
    void recalculatePath();

    /**
     * Starts dragging a control point
     * @param mousePos Current mouse position
     * @param tolerance Distance tolerance for hit detection
     * @return True if a point was found to drag
     */
    bool startDraggingPoint(Vector2 mousePos, float tolerance = 8.0f);

    /**
     * Updates the position of the dragged control point
     * @param mousePos New mouse position
     */
    void updateDraggedPoint(Vector2 mousePos);

    /**
     * Stops dragging a control point
     */
    void stopDraggingPoint();

    /**
     * Checks if a control point is being dragged
     * @return True if a point is being dragged
     */
    bool isDraggingPoint() const;

    void setSelected(bool selected);
    bool getIsSelected() const;
    bool isMouseOver(Vector2 mousePos, float tolerance = 2.0f) const;

    GatePin* getSourcePin() const;
    GatePin* getDestPin() const;
    bool getState() const;

private:
    /**
     * Helper function to draw the wire as a series of connected line segments.
     * @param points Vector of control points
     * @param color The color to draw the wire with.
     * @param thickness The thickness of the wire.
     */
    void drawWirePath(const std::vector<Vector2>& points, Color color, float thickness) const;

public:
    /**
     * Called when a LogicGate connected to this wire is being deleted.
     * If the deletedGate is the parent of either the source or destination pin of this wire,
     * the corresponding pin pointer in this wire is effectively nullified (or the wire should be marked for deletion).
     * @param deletedGate Pointer to the gate that is being deleted.
     */
    void onGateDeleted(LogicGate* deletedGate);
};

#endif // WIRE_H