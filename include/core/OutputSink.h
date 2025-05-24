#ifndef OUTPUT_SINK_H
#define OUTPUT_SINK_H

#include "core/LogicGate.h"
#include <string>

/** Visual style configuration for output sinks */
struct VisualStyle {
    Color fillOff;
    Color fillOn;
    Color textColor;
    Color outlineColor;

    VisualStyle(Color fillOff = DARKGRAY, Color fillOn = RED,
                Color text = WHITE, Color outline = BLACK)
        : fillOff(fillOff), fillOn(fillOn), textColor(text), outlineColor(outline) {}
};

/**
 * Output sink component that displays circuit output.
 * Shows the state of connected input signals visually.
 */
class OutputSink : public LogicGate {
public:
    /**
     * Constructs an output sink.
     * @param id Unique identifier
     * @param pos Position in world coordinates
     * @param radius Radius of the circular component
     * @param label Display label
     */
    OutputSink(std::string id, Vector2 pos, float radius, std::string label);
    ~OutputSink() override;

    void evaluate() override;
    void draw() override;

    /** Returns true if the sink is currently active (receiving a high signal) */
    bool isActive() const;

private:
    float radius_;
    std::string label_;
    bool active_;
    VisualStyle style_;
};

#endif // OUTPUT_SINK_H
