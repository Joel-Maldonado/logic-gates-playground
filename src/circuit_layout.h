#ifndef CIRCUIT_LAYOUT_H
#define CIRCUIT_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>
#include "logic.h"

typedef struct {
    const char *name;
    NodeType type;
    uint8_t input_count;
    uint8_t output_count;
    uint8_t _padding[2];
} CircuitLayoutNode;

typedef struct {
    uint32_t source_node_index;
    uint32_t sink_node_index;
    uint8_t source_pin_index;
    uint8_t sink_pin_index;
    uint8_t _padding[2];
} CircuitLayoutEdge;

bool circuit_layout_resolve_positions(
    const CircuitLayoutNode *nodes,
    uint32_t node_count,
    const CircuitLayoutEdge *edges,
    uint32_t edge_count,
    Vector2 *positions
);

#endif // CIRCUIT_LAYOUT_H
