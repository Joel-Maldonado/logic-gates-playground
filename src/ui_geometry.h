#ifndef UI_GEOMETRY_H
#define UI_GEOMETRY_H

#include "logic.h"

typedef struct {
    const LogicPin *source;
    const LogicPin *sink;
    Vector2 start;
    Vector2 bend_a;
    Vector2 bend_b;
    Vector2 end;
} UiWirePath;

Vector2 ui_input_pin_position(const LogicPin *pin);
Vector2 ui_output_pin_position(const LogicPin *pin);
Vector2 ui_pin_position(const LogicPin *pin);
UiWirePath ui_orthogonal_wire_path(Vector2 start, Vector2 end);
float ui_point_to_wire_distance(Vector2 point, UiWirePath path);
bool ui_find_incoming_wire_path(const LogicGraph *graph, const LogicPin *sink_pin, UiWirePath *path);

#endif // UI_GEOMETRY_H
