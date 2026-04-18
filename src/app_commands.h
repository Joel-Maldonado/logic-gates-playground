#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H

#include "app.h"

void app_queue_command(AppContext *app, EditorCommand command);
bool app_pop_command(AppContext *app, EditorCommand *command);
void app_handle_command(AppContext *app, EditorCommand command);
void app_cancel_interaction(AppContext *app);
void app_step_simulation(AppContext *app);
void app_update_simulation(AppContext *app);
void app_reset_simulation(AppContext *app);
bool app_delete_selected_node(AppContext *app);
bool app_delete_selected_wire(AppContext *app);
void app_select_wire_by_sink(AppContext *app, LogicPin *sink);
bool app_select_next_node(AppContext *app, int direction);
bool app_move_selected_node(AppContext *app, int grid_dx, int grid_dy);
LogicNode* app_create_node_for_tool(AppContext *app, AppTool tool);
bool app_select_node_by_index(AppContext *app, uint32_t node_index);
bool app_activate_pin_by_index(AppContext *app, uint32_t node_index, bool is_output_pin, uint8_t pin_index);
bool app_select_truth_row_by_index(AppContext *app, uint32_t row_index);
bool app_begin_wire_drag(AppContext *app, LogicPin *pin, Vector2 pointer_pos);
void app_update_wire_drag(AppContext *app, LogicPin *hover_pin, Vector2 pointer_pos);
bool app_commit_wire_drag(AppContext *app, LogicPin *pin);
void app_cancel_wire_drag(AppContext *app);
bool app_connect_pins(AppContext *app, LogicPin *first_pin, LogicPin *second_pin);
AppTool app_tool_from_node_type(NodeType type);
NodeType app_node_type_for_tool(AppTool tool);
bool app_tool_places_node(AppTool tool);
const char* app_mode_label(AppMode mode);
const char* app_tool_label(AppTool tool);

#endif // APP_COMMANDS_H
