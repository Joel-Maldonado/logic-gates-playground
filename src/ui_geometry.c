#include "ui_geometry.h"
#include "app_canvas.h"
#include <math.h>

static const LogicNet *ui_find_incoming_net(const LogicGraph *graph, const LogicPin *sink_pin) {
    uint32_t net_index;

    if (!graph || !sink_pin) {
        return NULL;
    }

    for (net_index = 0U; net_index < graph->net_count; net_index++) {
        uint8_t sink_index;

        for (sink_index = 0U; sink_index < graph->nets[net_index].sink_count; sink_index++) {
            if (graph->nets[net_index].sinks[sink_index] == sink_pin) {
                return &graph->nets[net_index];
            }
        }
    }

    return NULL;
}

static float ui_point_segment_distance(Vector2 point, Vector2 start, Vector2 end) {
    float dx;
    float dy;
    float length_squared;
    float t;
    float closest_x;
    float closest_y;

    dx = end.x - start.x;
    dy = end.y - start.y;
    length_squared = (dx * dx) + (dy * dy);
    if (length_squared < 0.0001f) {
        dx = point.x - start.x;
        dy = point.y - start.y;
        return sqrtf((dx * dx) + (dy * dy));
    }

    t = (((point.x - start.x) * dx) + ((point.y - start.y) * dy)) / length_squared;
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    closest_x = start.x + (t * dx);
    closest_y = start.y + (t * dy);
    dx = point.x - closest_x;
    dy = point.y - closest_y;
    return sqrtf((dx * dx) + (dy * dy));
}

Vector2 ui_input_pin_position(const LogicPin *pin) {
    return (Vector2){
        pin->node->pos.x,
        pin->node->pos.y + app_node_pin_offset_y(pin->node, false, pin->index)
    };
}

Vector2 ui_output_pin_position(const LogicPin *pin) {
    return (Vector2){
        pin->node->pos.x + pin->node->rect.width,
        pin->node->pos.y + app_node_pin_offset_y(pin->node, true, pin->index)
    };
}

Vector2 ui_pin_position(const LogicPin *pin) {
    if (pin->index < pin->node->input_count && &pin->node->inputs[pin->index] == pin) {
        return ui_input_pin_position(pin);
    }

    return ui_output_pin_position(pin);
}

UiWirePath ui_orthogonal_wire_path(Vector2 start, Vector2 end) {
    UiWirePath path;

    path.source = NULL;
    path.sink = NULL;
    path.start = start;
    path.bend_a = (Vector2){ (start.x + end.x) * 0.5f, start.y };
    path.bend_b = (Vector2){ (start.x + end.x) * 0.5f, end.y };
    path.end = end;
    return path;
}

float ui_point_to_wire_distance(Vector2 point, UiWirePath path) {
    float distance;
    float candidate;

    distance = ui_point_segment_distance(point, path.start, path.bend_a);
    candidate = ui_point_segment_distance(point, path.bend_a, path.bend_b);
    if (candidate < distance) {
        distance = candidate;
    }
    candidate = ui_point_segment_distance(point, path.bend_b, path.end);
    if (candidate < distance) {
        distance = candidate;
    }
    return distance;
}

bool ui_find_incoming_wire_path(const LogicGraph *graph, const LogicPin *sink_pin, UiWirePath *path) {
    const LogicNet *incoming;

    incoming = ui_find_incoming_net(graph, sink_pin);
    if (!incoming || !incoming->source) {
        return false;
    }

    if (path) {
        *path = ui_orthogonal_wire_path(ui_output_pin_position(incoming->source), ui_input_pin_position(sink_pin));
        path->source = incoming->source;
        path->sink = sink_pin;
    }

    return true;
}
