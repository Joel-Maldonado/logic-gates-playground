#ifndef APP_H
#define APP_H

#include "logic.h"

typedef enum {
    MODE_BUILD,
    MODE_COMPARE
} AppMode;

typedef enum {
    APP_TOOL_SELECT,
    APP_TOOL_INPUT,
    APP_TOOL_OUTPUT,
    APP_TOOL_AND,
    APP_TOOL_OR,
    APP_TOOL_NOT,
    APP_TOOL_XOR,
    APP_TOOL_CLOCK
} AppTool;

typedef enum {
    APP_PANEL_CANVAS,
    APP_PANEL_TRUTH_TABLE
} AppPanelFocus;

typedef enum {
    APP_COMPARE_NO_TARGET,
    APP_COMPARE_EQUIVALENT,
    APP_COMPARE_MISMATCH
} AppCompareStatus;

typedef enum {
    EDITOR_COMMAND_NONE,
    EDITOR_COMMAND_MODE_BUILD,
    EDITOR_COMMAND_MODE_COMPARE,
    EDITOR_COMMAND_TOOL_SELECT,
    EDITOR_COMMAND_TOOL_INPUT,
    EDITOR_COMMAND_TOOL_OUTPUT,
    EDITOR_COMMAND_TOOL_AND,
    EDITOR_COMMAND_TOOL_OR,
    EDITOR_COMMAND_TOOL_NOT,
    EDITOR_COMMAND_TOOL_XOR,
    EDITOR_COMMAND_TOOL_CLOCK,
    EDITOR_COMMAND_SIM_TOGGLE,
    EDITOR_COMMAND_SIM_STEP,
    EDITOR_COMMAND_SIM_RESET,
    EDITOR_COMMAND_CANCEL,
    EDITOR_COMMAND_DELETE_SELECTION,
    EDITOR_COMMAND_SELECT_NEXT_NODE,
    EDITOR_COMMAND_SELECT_PREVIOUS_NODE,
    EDITOR_COMMAND_MOVE_SELECTION_LEFT,
    EDITOR_COMMAND_MOVE_SELECTION_RIGHT,
    EDITOR_COMMAND_MOVE_SELECTION_UP,
    EDITOR_COMMAND_MOVE_SELECTION_DOWN,
    EDITOR_COMMAND_SELECT_PREVIOUS_ROW,
    EDITOR_COMMAND_SELECT_NEXT_ROW
} EditorCommand;

#define MAX_KMAP_GROUPS 8
#define APP_SOURCE_PATH_MAX 512
#define APP_STATUS_MESSAGE_MAX 128
#define APP_CANVAS_MIN_ZOOM 0.4f
#define APP_CANVAS_MAX_ZOOM 3.0f

typedef struct {
    char term[16];
    Color color;
    uint8_t cell_mask;
    uint8_t _padding[3];
} KMapGroup;

#define WAVEFORM_SAMPLES 100
#define APP_PENDING_COMMANDS 32

typedef struct {
    LogicNode *selected_node;
    LogicValue live_output;
    uint32_t live_row_index;
    bool row_valid;
    bool output_valid;
    uint8_t _padding[6];
} ViewContext;

typedef struct {
    LogicGraph graph;
    LogicValue waveforms[MAX_NODES][WAVEFORM_SAMPLES];

    TruthTable *current_table;
    char *current_expression;
    LogicNode *drag_node;
    LogicNode *selected_node;
    LogicPin *active_pin;
    LogicPin *wire_drag_pin;
    LogicPin *wire_hover_pin;
    LogicPin *selected_wire_sink;
    char *simplified_expression;
    LogicGraph *target_graph;
    LogicNode *divergence_node;
    EditorCommand pending_commands[APP_PENDING_COMMANDS];

    double last_tick_time;
    Vector2 drag_offset;
    Vector2 wire_drag_pos;
    Vector2 canvas_origin;

    KMapGroup kmap_groups[MAX_KMAP_GROUPS];
    ViewContext view_ctx;

    AppMode mode;
    AppTool active_tool;
    AppPanelFocus focused_panel;
    AppCompareStatus compare_status;
    float sim_speed; // Ticks per second
    float canvas_zoom;
    uint32_t waveform_index;
    uint32_t selected_row; // index into TruthTable row
    uint32_t first_failing_row;
    uint8_t pending_command_count;
    uint8_t kmap_group_count;
    char source_path[APP_SOURCE_PATH_MAX];
    char source_status[APP_STATUS_MESSAGE_MAX];

    bool wiring_active;
    bool wire_drag_active;
    bool wire_drag_replacing_sink;
    bool sim_active;
    bool comparison_equivalent;
    bool source_live_reload;
    uint8_t _padding[12];
} AppContext;

void app_init(AppContext *app);
void app_update_logic(AppContext *app);
void app_update_kmap_grouping(AppContext *app);
LogicNode* app_add_node(AppContext *app, NodeType type, Vector2 pos);
LogicNode* app_add_named_node(AppContext *app, NodeType type, const char *name, Vector2 pos);
void app_set_mode(AppContext *app, AppMode mode);
void app_set_tool(AppContext *app, AppTool tool);
void app_set_panel_focus(AppContext *app, AppPanelFocus panel);
void app_select_row(AppContext *app, uint32_t row_index);
void app_compare_with_target(AppContext *app, LogicGraph *target);
void app_compute_view_context(AppContext *app);
void app_apply_selected_row_to_inputs(AppContext *app);
bool app_toggle_input_value(AppContext *app, LogicNode *node);
char* app_get_node_explanation(AppContext *app, LogicNode *node);
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
void app_clear_graph(AppContext *app);
void app_set_source_path(AppContext *app, const char *path);
void app_set_source_status(AppContext *app, const char *status);
LogicNode* app_create_node_for_tool(AppContext *app, AppTool tool);
bool app_select_node_by_index(AppContext *app, uint32_t node_index);
bool app_activate_pin_by_index(AppContext *app, uint32_t node_index, bool is_output_pin, uint8_t pin_index);
bool app_select_truth_row_by_index(AppContext *app, uint32_t row_index);
bool app_begin_wire_drag(AppContext *app, LogicPin *pin, Vector2 pointer_pos);
void app_update_wire_drag(AppContext *app, LogicPin *hover_pin, Vector2 pointer_pos);
bool app_commit_wire_drag(AppContext *app, LogicPin *pin);
void app_cancel_wire_drag(AppContext *app);
bool app_connect_pins(AppContext *app, LogicPin *first_pin, LogicPin *second_pin);
Vector2 app_snap_node_position(Vector2 position, NodeType type);
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
Camera2D app_canvas_camera(const AppContext *app, Rectangle canvas_rect);
Vector2 app_canvas_screen_to_world(const AppContext *app, Rectangle canvas_rect, Vector2 screen_pos);
Vector2 app_canvas_world_to_screen(const AppContext *app, Rectangle canvas_rect, Vector2 world_pos);
void app_pan_canvas(AppContext *app, Vector2 screen_delta);
void app_zoom_canvas_at(AppContext *app, Rectangle canvas_rect, Vector2 screen_anchor, float zoom_factor);
AppTool app_tool_from_node_type(NodeType type);
NodeType app_node_type_for_tool(AppTool tool);
bool app_tool_places_node(AppTool tool);
const char* app_mode_label(AppMode mode);
const char* app_tool_label(AppTool tool);

#endif // APP_H
