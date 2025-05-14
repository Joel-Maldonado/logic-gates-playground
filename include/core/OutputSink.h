#ifndef OUTPUT_SINK_H
#define OUTPUT_SINK_H

#include "core/LogicGate.h"
#include <string>

struct VisualStyle {
    Color fillOff;
    Color fillOn;
    Color textColor;
    Color outlineColor;

    VisualStyle(Color fillOff = DARKGRAY, Color fillOn = RED,
                Color text = WHITE, Color outline = BLACK)
        : fillOff(fillOff), fillOn(fillOn), textColor(text), outlineColor(outline) {}
};

class OutputSink : public LogicGate {
public:
    OutputSink(std::string id, Vector2 pos, float radius, std::string label);
    ~OutputSink() override;

    void evaluate() override;
    void draw() override;

    bool isActive() const;

private:
    float radius;
    std::string label;
    bool active;
    VisualStyle style;
};

#endif // OUTPUT_SINK_H
