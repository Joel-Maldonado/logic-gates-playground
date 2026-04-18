#ifndef APP_CANVAS_H
#define APP_CANVAS_H

#include "app.h"

#define APP_GRID_SIZE 20.0f

void app_node_dimensions(NodeType type, int *width, int *height);
float app_node_pin_offset_y(const LogicNode *node, bool is_output_pin, uint8_t pin_index);
Vector2 app_default_node_position(const AppContext *app, NodeType type);
Vector2 app_snap_node_position(Vector2 position, NodeType type);
Vector2 app_snap_live_node_position(const AppContext *app, const LogicNode *node, Vector2 position);
float app_canvas_clamp_zoom(float zoom);
Vector2 app_canvas_screen_to_world_at(Vector2 origin, float zoom, Rectangle canvas_rect, Vector2 screen_pos);
Vector2 app_canvas_world_to_screen_at(Vector2 origin, float zoom, Rectangle canvas_rect, Vector2 world_pos);
Vector2 app_canvas_origin_after_pan(Vector2 origin, float zoom, Vector2 screen_delta);
Vector2 app_canvas_origin_after_zoom(
    Vector2 origin,
    float current_zoom,
    float new_zoom,
    Rectangle canvas_rect,
    Vector2 screen_anchor
);
void app_reset_canvas_view(AppContext *app);
bool app_frame_graph_in_canvas(AppContext *app, Rectangle canvas_rect);
Camera2D app_canvas_camera(const AppContext *app, Rectangle canvas_rect);
Vector2 app_canvas_screen_to_world(const AppContext *app, Rectangle canvas_rect, Vector2 screen_pos);
Vector2 app_canvas_world_to_screen(const AppContext *app, Rectangle canvas_rect, Vector2 world_pos);
void app_pan_canvas(AppContext *app, Vector2 screen_delta);
void app_zoom_canvas_at(AppContext *app, Rectangle canvas_rect, Vector2 screen_anchor, float zoom_factor);

#endif // APP_CANVAS_H
