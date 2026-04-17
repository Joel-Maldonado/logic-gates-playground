#ifndef OUTPUT_SINK_H
#define OUTPUT_SINK_H

#include "core/LogicGate.h"
#include <string>

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

    /** Returns true if the sink is currently active (receiving a high signal) */
    bool isActive() const;

private:
    bool active_;
};

#endif // OUTPUT_SINK_H
