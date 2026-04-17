#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_SIZE 20.0f
#define APP_NAME_BUFFER_SIZE 32

static uint32_t app_count_nodes_of_type(const AppContext *app, NodeType type);
void app_cancel_wire_drag(AppContext *app);

static const char *app_node_name(NodeType type, int *width, int *height) {
    *width = 80;
    *height = 60;

    if (type == NODE_INPUT) {
        *width = 60;
        *height = 40;
        return "IN";
    }
    if (type == NODE_OUTPUT) {
        *width = 60;
        *height = 40;
        return "OUT";
    }
    if (type == NODE_GATE_AND) {
        return "AND";
    }
    if (type == NODE_GATE_OR) {
        return "OR";
    }
    if (type == NODE_GATE_NOT) {
        *width = 60;
        *height = 40;
        return "NOT";
    }
    if (type == NODE_GATE_XOR) {
        return "XOR";
    }
    if (type == NODE_GATE_NAND) {
        return "NAND";
    }
    if (type == NODE_GATE_NOR) {
        return "NOR";
    }
    if (type == NODE_GATE_DFF) {
        return "DFF";
    }
    if (type == NODE_GATE_LATCH) {
        return "LATCH";
    }

    *width = 60;
    *height = 40;
    return "CLK";
}

Vector2 app_snap_node_position(Vector2 position, NodeType type) {
    int width;
    int height;
    float center_y;

    app_node_name(type, &width, &height);

    center_y = position.y + ((float)height / 2.0f);
    return (Vector2){
        (float)((int)(position.x / GRID_SIZE) * (int)GRID_SIZE),
        (float)((int)(center_y / GRID_SIZE) * (int)GRID_SIZE) - ((float)height / 2.0f)
    };
}

float app_canvas_clamp_zoom(float zoom) {
    if (zoom < APP_CANVAS_MIN_ZOOM) {
        return APP_CANVAS_MIN_ZOOM;
    }
    if (zoom > APP_CANVAS_MAX_ZOOM) {
        return APP_CANVAS_MAX_ZOOM;
    }

    return zoom;
}

Vector2 app_canvas_screen_to_world_at(Vector2 origin, float zoom, Rectangle canvas_rect, Vector2 screen_pos) {
    float safe_zoom;

    safe_zoom = app_canvas_clamp_zoom(zoom);
    return (Vector2){
        origin.x + ((screen_pos.x - canvas_rect.x) / safe_zoom),
        origin.y + ((screen_pos.y - canvas_rect.y) / safe_zoom)
    };
}

Vector2 app_canvas_world_to_screen_at(Vector2 origin, float zoom, Rectangle canvas_rect, Vector2 world_pos) {
    float safe_zoom;

    safe_zoom = app_canvas_clamp_zoom(zoom);
    return (Vector2){
        canvas_rect.x + ((world_pos.x - origin.x) * safe_zoom),
        canvas_rect.y + ((world_pos.y - origin.y) * safe_zoom)
    };
}

Vector2 app_canvas_origin_after_pan(Vector2 origin, float zoom, Vector2 screen_delta) {
    float safe_zoom;

    safe_zoom = app_canvas_clamp_zoom(zoom);
    return (Vector2){
        origin.x - (screen_delta.x / safe_zoom),
        origin.y - (screen_delta.y / safe_zoom)
    };
}

Vector2 app_canvas_origin_after_zoom(
    Vector2 origin,
    float current_zoom,
    float new_zoom,
    Rectangle canvas_rect,
    Vector2 screen_anchor
) {
    Vector2 anchor_world;
    float clamped_zoom;

    anchor_world = app_canvas_screen_to_world_at(origin, current_zoom, canvas_rect, screen_anchor);
    clamped_zoom = app_canvas_clamp_zoom(new_zoom);
    return (Vector2){
        anchor_world.x - ((screen_anchor.x - canvas_rect.x) / clamped_zoom),
        anchor_world.y - ((screen_anchor.y - canvas_rect.y) / clamped_zoom)
    };
}

void app_reset_canvas_view(AppContext *app) {
    if (!app) {
        return;
    }

    app->canvas_origin = (Vector2){ 0.0f, 0.0f };
    app->canvas_zoom = 1.0f;
}

Camera2D app_canvas_camera(const AppContext *app, Rectangle canvas_rect) {
    Camera2D camera;

    memset(&camera, 0, sizeof(camera));
    if (!app) {
        return camera;
    }

    camera.offset = (Vector2){ canvas_rect.x, canvas_rect.y };
    camera.target = app->canvas_origin;
    camera.rotation = 0.0f;
    camera.zoom = app_canvas_clamp_zoom(app->canvas_zoom);
    return camera;
}

Vector2 app_canvas_screen_to_world(const AppContext *app, Rectangle canvas_rect, Vector2 screen_pos) {
    if (!app) {
        return (Vector2){ 0.0f, 0.0f };
    }

    return app_canvas_screen_to_world_at(app->canvas_origin, app->canvas_zoom, canvas_rect, screen_pos);
}

Vector2 app_canvas_world_to_screen(const AppContext *app, Rectangle canvas_rect, Vector2 world_pos) {
    if (!app) {
        return (Vector2){ 0.0f, 0.0f };
    }

    return app_canvas_world_to_screen_at(app->canvas_origin, app->canvas_zoom, canvas_rect, world_pos);
}

void app_pan_canvas(AppContext *app, Vector2 screen_delta) {
    if (!app) {
        return;
    }

    app->canvas_origin = app_canvas_origin_after_pan(app->canvas_origin, app->canvas_zoom, screen_delta);
}

void app_zoom_canvas_at(AppContext *app, Rectangle canvas_rect, Vector2 screen_anchor, float zoom_factor) {
    float next_zoom;
    float delta;

    if (!app || zoom_factor <= 0.0f) {
        return;
    }

    next_zoom = app_canvas_clamp_zoom(app->canvas_zoom * zoom_factor);
    delta = next_zoom - app->canvas_zoom;
    if (delta < 0.0f) {
        delta = -delta;
    }
    if (delta < 0.0001f) {
        return;
    }

    app->canvas_origin = app_canvas_origin_after_zoom(
        app->canvas_origin,
        app->canvas_zoom,
        next_zoom,
        canvas_rect,
        screen_anchor
    );
    app->canvas_zoom = next_zoom;
}

static bool app_pin_is_input(const LogicPin *pin) {
    return pin &&
        pin->node &&
        pin->index < pin->node->input_count &&
        &pin->node->inputs[pin->index] == pin;
}

static bool app_pin_is_output(const LogicPin *pin) {
    return pin &&
        pin->node &&
        pin->index < pin->node->output_count &&
        &pin->node->outputs[pin->index] == pin;
}

static bool app_sink_has_connection(const AppContext *app, const LogicPin *pin) {
    uint32_t i;

    if (!app_pin_is_input(pin)) {
        return false;
    }

    for (i = 0; i < app->graph.net_count; i++) {
        uint8_t sink_index;

        for (sink_index = 0; sink_index < app->graph.nets[i].sink_count; sink_index++) {
            if (app->graph.nets[i].sinks[sink_index] == pin) {
                return true;
            }
        }
    }

    return false;
}

static LogicNode *app_primary_output_node(AppContext *app) {
    uint32_t i;

    for (i = 0; i < app->graph.node_count; i++) {
        if (app->graph.nodes[i].type == NODE_OUTPUT) {
            return &app->graph.nodes[i];
        }
    }

    return NULL;
}

static void app_reset_waveforms(AppContext *app) {
    app->waveform_index = 0U;
    memset(app->waveforms, 0, sizeof(app->waveforms));
}

static void app_build_input_name(uint32_t index, char *buffer, size_t buffer_size) {
    char letter;
    uint32_t suffix;

    letter = (char)('A' + (char)(index % 26U));
    suffix = index / 26U;
    if (suffix == 0U) {
        snprintf(buffer, buffer_size, "%c", letter);
        return;
    }

    snprintf(buffer, buffer_size, "%c%u", letter, suffix);
}

static void app_build_output_name(uint32_t index, char *buffer, size_t buffer_size) {
    if (index == 0U) {
        snprintf(buffer, buffer_size, "Z");
        return;
    }

    snprintf(buffer, buffer_size, "Z%u", index);
}

static const char *app_gate_name_prefix(NodeType type) {
    switch (type) {
        case NODE_INPUT:
            return "IN";
        case NODE_OUTPUT:
            return "OUT";
        case NODE_GATE_AND:
            return "AND";
        case NODE_GATE_OR:
            return "OR";
        case NODE_GATE_NOT:
            return "NOT";
        case NODE_GATE_XOR:
            return "XOR";
        case NODE_GATE_NAND:
            return "NAND";
        case NODE_GATE_NOR:
            return "NOR";
        case NODE_GATE_DFF:
            return "DFF";
        case NODE_GATE_LATCH:
            return "LATCH";
        case NODE_GATE_CLOCK:
            return "CLK";
        default:
            return "NODE";
    }
}

static void app_default_node_name(const AppContext *app, NodeType type, char *buffer, size_t buffer_size) {
    uint32_t count;

    count = app_count_nodes_of_type(app, type);
    if (type == NODE_INPUT) {
        app_build_input_name(count, buffer, buffer_size);
        return;
    }
    if (type == NODE_OUTPUT) {
        app_build_output_name(count, buffer, buffer_size);
        return;
    }

    snprintf(buffer, buffer_size, "%s%u", app_gate_name_prefix(type), count + 1U);
}

static LogicValue app_node_waveform_value(const LogicNode *node) {
    if (node->type == NODE_OUTPUT && node->input_count > 0) {
        return node->inputs[0].value;
    }
    if (node->output_count > 0) {
        return node->outputs[0].value;
    }
    if (node->input_count > 0) {
        return node->inputs[0].value;
    }
    return LOGIC_UNKNOWN;
}

static uint32_t app_count_nodes_of_type(const AppContext *app, NodeType type) {
    uint32_t count;
    uint32_t i;

    count = 0U;
    for (i = 0; i < app->graph.node_count; i++) {
        if (app->graph.nodes[i].type == type) {
            count++;
        }
    }

    return count;
}

static Vector2 app_default_position_for_type(const AppContext *app, NodeType type) {
    uint32_t count;

    count = app_count_nodes_of_type(app, type);
    switch (type) {
        case NODE_INPUT:
            return (Vector2){ 140.0f, 160.0f + ((float)count * 100.0f) };
        case NODE_OUTPUT:
            return (Vector2){ 520.0f, 220.0f + ((float)count * 100.0f) };
        case NODE_GATE_AND:
        case NODE_GATE_OR:
        case NODE_GATE_NOT:
        case NODE_GATE_XOR:
        case NODE_GATE_NAND:
        case NODE_GATE_NOR:
        case NODE_GATE_DFF:
        case NODE_GATE_LATCH:
        case NODE_GATE_CLOCK:
            return (Vector2){ 320.0f, 200.0f + ((float)count * 100.0f) };
        default:
            return (Vector2){ 320.0f, 200.0f };
    }
}

static void app_record_waveforms(AppContext *app) {
    uint32_t i;

    for (i = 0; i < app->graph.node_count; i++) {
        LogicNode *node;

        node = &app->graph.nodes[i];
        if (node->type == (NodeType)-1) {
            continue;
        }
        app->waveforms[i][app->waveform_index] = app_node_waveform_value(node);
    }

    app->waveform_index = (app->waveform_index + 1U) % WAVEFORM_SAMPLES;
}

void app_apply_selected_row_to_inputs(AppContext *app) {
    uint32_t cols;
    uint8_t i;

    if (!app || !app->current_table) {
        return;
    }

    if (app->selected_row >= app->current_table->row_count) {
        app->selected_row = 0;
    }

    cols = (uint32_t)app->current_table->input_count + (uint32_t)app->current_table->output_count;
    for (i = 0; i < app->current_table->input_count; i++) {
        app->current_table->inputs[i]->outputs[0].value =
            app->current_table->data[(app->selected_row * cols) + (uint32_t)i];
    }

    logic_evaluate(&app->graph);
    app_compute_view_context(app);
}

bool app_toggle_input_value(AppContext *app, LogicNode *node) {
    LogicValue next;

    if (!app || !node || node->type != NODE_INPUT || node->output_count == 0) {
        return false;
    }

    next = (node->outputs[0].value == LOGIC_HIGH) ? LOGIC_LOW : LOGIC_HIGH;
    node->outputs[0].value = next;
    logic_evaluate(&app->graph);
    app_compute_view_context(app);
    return true;
}

void app_compute_view_context(AppContext *app) {
    ViewContext *ctx;
    uint32_t row;
    uint8_t i;

    if (!app) {
        return;
    }

    ctx = &app->view_ctx;
    ctx->selected_node = app->selected_node;
    ctx->row_valid = false;
    ctx->output_valid = false;
    ctx->live_row_index = 0U;
    ctx->live_output = LOGIC_UNKNOWN;

    if (app->selected_node) {
        LogicNode *node = app->selected_node;
        if (node->type != (NodeType)-1) {
            if (node->type == NODE_OUTPUT && node->input_count > 0) {
                ctx->live_output = node->inputs[0].value;
                ctx->output_valid = true;
            } else if (node->output_count > 0) {
                ctx->live_output = node->outputs[0].value;
                ctx->output_valid = true;
            }
        }
    }

    if (!app->current_table || app->current_table->input_count == 0U) {
        return;
    }

    row = 0U;
    for (i = 0; i < app->current_table->input_count; i++) {
        LogicNode *in_node;
        uint8_t bit_index;

        in_node = app->current_table->inputs[i];
        if (!in_node || in_node->output_count == 0) {
            return;
        }
        if (in_node->outputs[0].value != LOGIC_LOW &&
            in_node->outputs[0].value != LOGIC_HIGH) {
            return;
        }
        bit_index = (uint8_t)(app->current_table->input_count - 1U - i);
        if (in_node->outputs[0].value == LOGIC_HIGH) {
            row |= (1U << bit_index);
        }
    }

    if (row < app->current_table->row_count) {
        ctx->live_row_index = row;
        ctx->row_valid = true;
    }
}

static void app_compare_if_needed(AppContext *app) {
    if (!app->target_graph) {
        app->comparison_equivalent = false;
        app->compare_status = APP_COMPARE_NO_TARGET;
        app->divergence_node = NULL;
        app->first_failing_row = 0U;
        return;
    }

    app_compare_with_target(app, app->target_graph);
}

void app_init(AppContext *app) {
    memset(app, 0, sizeof(AppContext));
    logic_init_graph(&app->graph);
    app->mode = MODE_BUILD;
    app->active_tool = APP_TOOL_SELECT;
    app->focused_panel = APP_PANEL_CANVAS;
    app->selected_row = 0;
    app->sim_speed = 2.0f;
    app->last_tick_time = 0.0;
    app->sim_active = false;
    app->compare_status = APP_COMPARE_NO_TARGET;
    app_reset_canvas_view(app);
    app_set_source_status(app, "Canvas editing");
}

void app_update_logic(AppContext *app) {
    LogicNode *output_node;
    LogicValue saved_inputs[MAX_PINS];
    LogicNode *saved_input_nodes[MAX_PINS];
    uint8_t saved_count;
    uint32_t i;

    app_reset_waveforms(app);
    output_node = app_primary_output_node(app);

    saved_count = 0U;
    for (i = 0; i < app->graph.node_count && saved_count < MAX_PINS; i++) {
        LogicNode *node = &app->graph.nodes[i];
        if (node->type == NODE_INPUT && node->output_count > 0) {
            saved_input_nodes[saved_count] = node;
            saved_inputs[saved_count] = node->outputs[0].value;
            saved_count++;
        }
    }

    if (app->current_table) {
        logic_free_truth_table(app->current_table);
    }
    app->current_table = logic_generate_truth_table(&app->graph);

    for (i = 0; i < saved_count; i++) {
        saved_input_nodes[i]->outputs[0].value = saved_inputs[i];
    }
    logic_evaluate(&app->graph);

    free(app->current_expression);
    app->current_expression = NULL;

    if (output_node) {
        app->current_expression = logic_generate_expression(&app->graph, output_node);
    }

    app_update_kmap_grouping(app);
    app_compute_view_context(app);
    app_compare_if_needed(app);
    app_record_waveforms(app);
}

LogicNode* app_add_node(AppContext *app, NodeType type, Vector2 pos) {
    return app_add_named_node(app, type, NULL, pos);
}

LogicNode* app_add_named_node(AppContext *app, NodeType type, const char *custom_name, Vector2 pos) {
    const char *resolved_name;
    char generated_name[APP_NAME_BUFFER_SIZE];
    int width;
    int height;
    LogicNode *node;

    if (!custom_name) {
        (void)app_node_name(type, &width, &height);
        app_default_node_name(app, type, generated_name, sizeof(generated_name));
        resolved_name = generated_name;
    } else {
        (void)app_node_name(type, &width, &height);
        resolved_name = custom_name;
    }

    node = logic_add_node(&app->graph, type, resolved_name);
    if (!node) {
        return NULL;
    }

    node->pos = pos;
    node->rect = (Rectangle){ pos.x, pos.y, (float)width, (float)height };

    /* Fresh inputs start HIGH so the circuit reads "1" rather than "?" — a
       plain "?" looks like a broken tool to a student. */
    if (type == NODE_INPUT && node->output_count > 0) {
        node->outputs[0].value = LOGIC_HIGH;
    }

    return node;
}

void app_set_mode(AppContext *app, AppMode mode) {
    app->mode = mode;
    app_compute_view_context(app);
    app_compare_if_needed(app);
}

void app_set_tool(AppContext *app, AppTool tool) {
    app->active_tool = tool;
    app->wiring_active = false;
    app->active_pin = NULL;
    app_cancel_wire_drag(app);
}

void app_set_panel_focus(AppContext *app, AppPanelFocus panel) {
    app->focused_panel = panel;
}

void app_select_row(AppContext *app, uint32_t row_index) {
    if (!app->current_table) {
        return;
    }
    if (row_index >= app->current_table->row_count) {
        return;
    }

    app->selected_row = row_index;
    app->focused_panel = APP_PANEL_TRUTH_TABLE;
}

void app_compare_with_target(AppContext *app, LogicGraph *target) {
    TruthTable *target_table;
    uint32_t cols;
    uint32_t target_cols;
    uint32_t r;
    uint8_t o;

    app->target_graph = target;
    app->comparison_equivalent = true;
    app->compare_status = APP_COMPARE_EQUIVALENT;
    app->divergence_node = NULL;
    app->first_failing_row = 0;

    target_table = logic_generate_truth_table(target);
    if (!target_table || !app->current_table) {
        if (target_table) {
            logic_free_truth_table(target_table);
        }
        app->compare_status = APP_COMPARE_NO_TARGET;
        app->comparison_equivalent = false;
        return;
    }

    cols = (uint32_t)app->current_table->input_count + (uint32_t)app->current_table->output_count;
    target_cols = (uint32_t)target_table->input_count + (uint32_t)target_table->output_count;

    if (app->current_table->input_count != target_table->input_count ||
        app->current_table->output_count != target_table->output_count) {
        app->comparison_equivalent = false;
        app->compare_status = APP_COMPARE_MISMATCH;
    } else {
        for (r = 0; r < app->current_table->row_count; r++) {
            bool row_match;

            row_match = true;
            for (o = 0; o < app->current_table->output_count; o++) {
                if (app->current_table->data[(r * cols) + (uint32_t)app->current_table->input_count + (uint32_t)o] !=
                    target_table->data[(r * target_cols) + (uint32_t)target_table->input_count + (uint32_t)o]) {
                    row_match = false;
                    break;
                }
            }
            if (!row_match) {
                app->comparison_equivalent = false;
                app->compare_status = APP_COMPARE_MISMATCH;
                app->first_failing_row = r;
                break;
            }
        }
    }

    logic_free_truth_table(target_table);
}

char* app_get_node_explanation(AppContext *app, LogicNode *node) {
    char *buf;
    LogicValue output_value;
    const char *output_text;

    (void)app;
    if (!node || node->type == (NodeType)-1) {
        return NULL;
    }

    buf = (char *)calloc(256, sizeof(char));
    if (!buf) {
        return NULL;
    }

    output_value = (node->output_count > 0) ? node->outputs[0].value : node->inputs[0].value;
    output_text = (output_value == LOGIC_HIGH) ? "1" : "0";

    if (node->type == NODE_GATE_AND) {
        snprintf(buf, 256, (output_value == LOGIC_HIGH)
            ? "Output is 1 because all inputs are 1."
            : "Output is 0 because at least one input is 0.");
    } else if (node->type == NODE_GATE_OR) {
        snprintf(buf, 256, (output_value == LOGIC_HIGH)
            ? "Output is 1 because at least one input is 1."
            : "Output is 0 because all inputs are 0.");
    } else if (node->type == NODE_GATE_NOT) {
        snprintf(buf, 256, "NOT gate inverts input to %s.", output_text);
    } else if (node->type == NODE_GATE_DFF) {
        if (output_value == LOGIC_ERROR) {
            snprintf(buf, 256, "POSSIBLE RACE CONDITION: Input D changed at the exact same tick as the rising edge of CLK.");
        } else {
            snprintf(buf, 256, "DFF stores state %s (latched on rising edge of CLK).", output_text);
        }
    } else if (node->type == NODE_GATE_CLOCK) {
        snprintf(buf, 256, "Clock toggles on every tick.");
    } else if (node->type == NODE_INPUT) {
        snprintf(buf, 256, "Input node fixed at %s.", output_text);
    } else if (node->type == NODE_OUTPUT) {
        snprintf(buf, 256, "Output reflects its incoming pin %s.", output_text);
    } else if (node->type == NODE_GATE_XOR) {
        snprintf(buf, 256, "XOR gate outputs 1 if inputs differ.");
    } else if (node->type == NODE_GATE_NAND) {
        snprintf(buf, 256, "NAND gate outputs 0 only if all inputs are 1.");
    } else if (node->type == NODE_GATE_NOR) {
        snprintf(buf, 256, "NOR gate outputs 1 only if all inputs are 0.");
    } else if (node->type == NODE_GATE_LATCH) {
        snprintf(buf, 256, "LATCH stores state while enabled.");
    }

    return buf;
}

void app_queue_command(AppContext *app, EditorCommand command) {
    if (!app || command == EDITOR_COMMAND_NONE) {
        return;
    }
    if (app->pending_command_count >= APP_PENDING_COMMANDS) {
        return;
    }

    app->pending_commands[app->pending_command_count++] = command;
}

bool app_pop_command(AppContext *app, EditorCommand *command) {
    uint8_t i;

    if (!app || !command || app->pending_command_count == 0) {
        return false;
    }

    *command = app->pending_commands[0];
    for (i = 1; i < app->pending_command_count; i++) {
        app->pending_commands[i - 1U] = app->pending_commands[i];
    }
    app->pending_command_count--;
    return true;
}

void app_cancel_interaction(AppContext *app) {
    app->wiring_active = false;
    app->active_pin = NULL;
    app->drag_node = NULL;
    app->selected_wire_sink = NULL;
    app_cancel_wire_drag(app);
    app->active_tool = APP_TOOL_SELECT;
}

void app_select_wire_by_sink(AppContext *app, LogicPin *sink) {
    if (!app) {
        return;
    }
    app->selected_wire_sink = sink;
    if (sink) {
        app->selected_node = NULL;
        app->drag_node = NULL;
        app_set_panel_focus(app, APP_PANEL_CANVAS);
    }
}

bool app_delete_selected_wire(AppContext *app) {
    LogicPin *sink;

    if (!app || !app->selected_wire_sink) {
        return false;
    }

    sink = app->selected_wire_sink;
    app->selected_wire_sink = NULL;

    if (!logic_disconnect_sink(&app->graph, sink)) {
        return false;
    }

    app_update_logic(app);
    return true;
}

void app_step_simulation(AppContext *app) {
    logic_tick(&app->graph);
    app_record_waveforms(app);
    app->last_tick_time = GetTime();
}

void app_reset_simulation(AppContext *app) {
    if (!app) {
        return;
    }

    app->sim_active = false;
    app->last_tick_time = 0.0;
    app->waveform_index = 0U;
    memset(app->waveforms, 0, sizeof(app->waveforms));
    logic_evaluate(&app->graph);
    app_compute_view_context(app);
}

void app_update_simulation(AppContext *app) {
    double now;
    double interval;

    if (!app->sim_active || app->sim_speed <= 0.0f) {
        return;
    }

    now = GetTime();
    interval = 1.0 / (double)app->sim_speed;
    if (app->last_tick_time <= 0.0) {
        app->last_tick_time = now;
    }

    while ((now - app->last_tick_time) >= interval) {
        logic_tick(&app->graph);
        app_record_waveforms(app);
        app->last_tick_time += interval;
    }
}

bool app_delete_selected_node(AppContext *app) {
    if (!app->selected_node) {
        return false;
    }

    if (app->active_pin && app->active_pin->node == app->selected_node) {
        app->active_pin = NULL;
        app->wiring_active = false;
    }
    if (app->wire_drag_pin && app->wire_drag_pin->node == app->selected_node) {
        app_cancel_wire_drag(app);
    }

    if (!logic_remove_node(&app->graph, app->selected_node)) {
        return false;
    }

    app->selected_node = NULL;
    app->selected_wire_sink = NULL;
    app->drag_node = NULL;
    app_update_logic(app);
    return true;
}

bool app_select_next_node(AppContext *app, int direction) {
    uint32_t i;
    uint32_t start_index;

    if (app->graph.node_count == 0) {
        return false;
    }

    if (direction >= 0) {
        start_index = 0;
        if (app->selected_node) {
            start_index = (uint32_t)(app->selected_node - app->graph.nodes) + 1U;
        }

        for (i = 0; i < app->graph.node_count; i++) {
            uint32_t index;

            index = (start_index + i) % app->graph.node_count;
            if (app->graph.nodes[index].type == (NodeType)-1) {
                continue;
            }
            app->selected_node = &app->graph.nodes[index];
            app->selected_wire_sink = NULL;
            app->focused_panel = APP_PANEL_CANVAS;
            return true;
        }
    } else {
        start_index = app->graph.node_count - 1U;
        if (app->selected_node) {
            start_index = (uint32_t)(app->selected_node - app->graph.nodes);
            start_index = (start_index == 0U) ? app->graph.node_count - 1U : start_index - 1U;
        }

        for (i = 0; i < app->graph.node_count; i++) {
            uint32_t index;

            index = (start_index + app->graph.node_count - i) % app->graph.node_count;
            if (app->graph.nodes[index].type == (NodeType)-1) {
                continue;
            }
            app->selected_node = &app->graph.nodes[index];
            app->selected_wire_sink = NULL;
            app->focused_panel = APP_PANEL_CANVAS;
            return true;
        }
    }

    return false;
}

bool app_move_selected_node(AppContext *app, int grid_dx, int grid_dy) {
    if (!app->selected_node) {
        return false;
    }

    app->selected_node->pos.x += (float)grid_dx * GRID_SIZE;
    app->selected_node->pos.y += (float)grid_dy * GRID_SIZE;
    app->selected_node->rect.x = app->selected_node->pos.x;
    app->selected_node->rect.y = app->selected_node->pos.y;
    app->focused_panel = APP_PANEL_CANVAS;
    return true;
}

LogicNode* app_create_node_for_tool(AppContext *app, AppTool tool) {
    LogicNode *node;
    NodeType type;

    if (!app_tool_places_node(tool)) {
        return NULL;
    }

    type = app_node_type_for_tool(tool);
    node = app_add_named_node(app, type, NULL, app_default_position_for_type(app, type));
    if (!node) {
        return NULL;
    }

    app->selected_node = node;
    app->selected_wire_sink = NULL;
    app->active_tool = APP_TOOL_SELECT;
    app_cancel_wire_drag(app);
    app_update_logic(app);
    app_set_panel_focus(app, APP_PANEL_CANVAS);
    return node;
}

bool app_select_node_by_index(AppContext *app, uint32_t node_index) {
    if (!app || node_index >= app->graph.node_count) {
        return false;
    }
    if (app->graph.nodes[node_index].type == (NodeType)-1) {
        return false;
    }

    app->selected_node = &app->graph.nodes[node_index];
    app->selected_wire_sink = NULL;
    app->drag_node = NULL;
    app_set_panel_focus(app, APP_PANEL_CANVAS);
    return true;
}

bool app_activate_pin_by_index(AppContext *app, uint32_t node_index, bool is_output_pin, uint8_t pin_index) {
    LogicNode *node;
    LogicPin *pin;

    if (!app || node_index >= app->graph.node_count) {
        return false;
    }

    node = &app->graph.nodes[node_index];
    if (node->type == (NodeType)-1) {
        return false;
    }

    if (is_output_pin) {
        if (pin_index >= node->output_count) {
            return false;
        }
        pin = &node->outputs[pin_index];
    } else {
        if (pin_index >= node->input_count) {
            return false;
        }
        pin = &node->inputs[pin_index];
    }

    app->active_tool = APP_TOOL_SELECT;
    app_set_panel_focus(app, APP_PANEL_CANVAS);
    app->selected_node = node;
    app->selected_wire_sink = NULL;

    if (!app->wiring_active) {
        app->wiring_active = true;
        app->active_pin = pin;
        app_cancel_wire_drag(app);
        return true;
    }

    if (!app_connect_pins(app, app->active_pin, pin)) {
        app->active_pin = pin;
        return false;
    }

    app->wiring_active = false;
    app->active_pin = NULL;
    return true;
}

bool app_connect_pins(AppContext *app, LogicPin *first_pin, LogicPin *second_pin) {
    LogicPin *source_pin;
    LogicPin *sink_pin;

    if (!app || !first_pin || !second_pin) {
        return false;
    }

    if (app_pin_is_output(first_pin) && app_pin_is_input(second_pin)) {
        source_pin = first_pin;
        sink_pin = second_pin;
    } else if (app_pin_is_output(second_pin) && app_pin_is_input(first_pin)) {
        source_pin = second_pin;
        sink_pin = first_pin;
    } else {
        return false;
    }

    if (!logic_connect(&app->graph, source_pin, sink_pin)) {
        return false;
    }

    app_update_logic(app);
    return true;
}

bool app_begin_wire_drag(AppContext *app, LogicPin *pin, Vector2 pointer_pos) {
    if (!app || !pin || (!app_pin_is_input(pin) && !app_pin_is_output(pin))) {
        return false;
    }

    app->active_tool = APP_TOOL_SELECT;
    app->selected_node = pin->node;
    app->selected_wire_sink = NULL;
    app->focused_panel = APP_PANEL_CANVAS;
    app->wiring_active = false;
    app->active_pin = NULL;
    app->wire_drag_active = true;
    app->wire_drag_pin = pin;
    app->wire_hover_pin = NULL;
    app->wire_drag_replacing_sink = false;
    app->wire_drag_pos = pointer_pos;
    return true;
}

void app_update_wire_drag(AppContext *app, LogicPin *hover_pin, Vector2 pointer_pos) {
    if (!app || !app->wire_drag_active) {
        return;
    }

    app->wire_drag_pos = pointer_pos;
    app->wire_hover_pin = hover_pin;
    app->wire_drag_replacing_sink =
        hover_pin &&
        app_pin_is_input(hover_pin) &&
        hover_pin != app->wire_drag_pin &&
        app_sink_has_connection(app, hover_pin);
}

bool app_commit_wire_drag(AppContext *app, LogicPin *pin) {
    bool connected;

    if (!app || !app->wire_drag_active || !app->wire_drag_pin) {
        return false;
    }

    connected = false;
    if (pin) {
        connected = app_connect_pins(app, app->wire_drag_pin, pin);
    }

    app_cancel_wire_drag(app);
    return connected;
}

void app_cancel_wire_drag(AppContext *app) {
    if (!app) {
        return;
    }

    app->wire_drag_active = false;
    app->wire_drag_pin = NULL;
    app->wire_hover_pin = NULL;
    app->wire_drag_replacing_sink = false;
    app->wire_drag_pos = (Vector2){ 0.0f, 0.0f };
}

bool app_select_truth_row_by_index(AppContext *app, uint32_t row_index) {
    if (!app || !app->current_table || row_index >= app->current_table->row_count) {
        return false;
    }

    app_select_row(app, row_index);
    return true;
}

void app_clear_graph(AppContext *app) {
    uint32_t i;

    if (!app) {
        return;
    }

    for (i = 0; i < app->graph.node_count; i++) {
        free(app->graph.nodes[i].name);
        app->graph.nodes[i].name = NULL;
    }

    logic_init_graph(&app->graph);
    if (app->current_table) {
        logic_free_truth_table(app->current_table);
        app->current_table = NULL;
    }
    free(app->current_expression);
    app->current_expression = NULL;
    free(app->simplified_expression);
    app->simplified_expression = NULL;
    app->drag_node = NULL;
    app->selected_node = NULL;
    app->selected_wire_sink = NULL;
    app->active_pin = NULL;
    app->wire_drag_pin = NULL;
    app->wire_hover_pin = NULL;
    app->wiring_active = false;
    app->wire_drag_active = false;
    app->wire_drag_replacing_sink = false;
    app->sim_active = false;
    app->last_tick_time = 0.0;
    app->waveform_index = 0;
    memset(app->waveforms, 0, sizeof(app->waveforms));
    app->selected_row = 0;
    app->kmap_group_count = 0;
    app->compare_status = APP_COMPARE_NO_TARGET;
    app->comparison_equivalent = false;
}

void app_set_source_path(AppContext *app, const char *path) {
    if (!app) {
        return;
    }

    if (!path) {
        app->source_path[0] = '\0';
        return;
    }

    snprintf(app->source_path, sizeof(app->source_path), "%s", path);
}

void app_set_source_status(AppContext *app, const char *status) {
    if (!app) {
        return;
    }

    if (!status) {
        app->source_status[0] = '\0';
        return;
    }

    snprintf(app->source_status, sizeof(app->source_status), "%s", status);
}

void app_handle_command(AppContext *app, EditorCommand command) {
    if (!app) {
        return;
    }

    switch (command) {
        case EDITOR_COMMAND_MODE_BUILD:
            app_set_mode(app, MODE_BUILD);
            break;
        case EDITOR_COMMAND_MODE_COMPARE:
            app_set_mode(app, MODE_COMPARE);
            break;
        case EDITOR_COMMAND_TOOL_SELECT:
            app_set_tool(app, APP_TOOL_SELECT);
            break;
        case EDITOR_COMMAND_TOOL_INPUT:
            app_set_tool(app, APP_TOOL_INPUT);
            break;
        case EDITOR_COMMAND_TOOL_OUTPUT:
            app_set_tool(app, APP_TOOL_OUTPUT);
            break;
        case EDITOR_COMMAND_TOOL_AND:
            app_set_tool(app, APP_TOOL_AND);
            break;
        case EDITOR_COMMAND_TOOL_OR:
            app_set_tool(app, APP_TOOL_OR);
            break;
        case EDITOR_COMMAND_TOOL_NOT:
            app_set_tool(app, APP_TOOL_NOT);
            break;
        case EDITOR_COMMAND_TOOL_XOR:
            app_set_tool(app, APP_TOOL_XOR);
            break;
        case EDITOR_COMMAND_TOOL_CLOCK:
            app_set_tool(app, APP_TOOL_CLOCK);
            break;
        case EDITOR_COMMAND_SIM_TOGGLE:
            app->sim_active = !app->sim_active;
            app->last_tick_time = GetTime();
            break;
        case EDITOR_COMMAND_SIM_STEP:
            app->sim_active = false;
            app_step_simulation(app);
            break;
        case EDITOR_COMMAND_SIM_RESET:
            app_reset_simulation(app);
            break;
        case EDITOR_COMMAND_CANCEL:
            app_cancel_interaction(app);
            break;
        case EDITOR_COMMAND_DELETE_SELECTION:
            if (app->selected_wire_sink) {
                app_delete_selected_wire(app);
            } else {
                app_delete_selected_node(app);
            }
            break;
        case EDITOR_COMMAND_SELECT_NEXT_NODE:
            app_select_next_node(app, 1);
            break;
        case EDITOR_COMMAND_SELECT_PREVIOUS_NODE:
            app_select_next_node(app, -1);
            break;
        case EDITOR_COMMAND_MOVE_SELECTION_LEFT:
            app_move_selected_node(app, -1, 0);
            break;
        case EDITOR_COMMAND_MOVE_SELECTION_RIGHT:
            app_move_selected_node(app, 1, 0);
            break;
        case EDITOR_COMMAND_MOVE_SELECTION_UP:
            app_move_selected_node(app, 0, -1);
            break;
        case EDITOR_COMMAND_MOVE_SELECTION_DOWN:
            app_move_selected_node(app, 0, 1);
            break;
        case EDITOR_COMMAND_SELECT_PREVIOUS_ROW:
            if (app->current_table && app->selected_row > 0U) {
                app_select_row(app, app->selected_row - 1U);
            }
            break;
        case EDITOR_COMMAND_SELECT_NEXT_ROW:
            if (app->current_table && app->selected_row + 1U < app->current_table->row_count) {
                app_select_row(app, app->selected_row + 1U);
            }
            break;
        case EDITOR_COMMAND_NONE:
            break;
        default:
            break;
    }
}

AppTool app_tool_from_node_type(NodeType type) {
    switch (type) {
        case NODE_INPUT:
            return APP_TOOL_INPUT;
        case NODE_OUTPUT:
            return APP_TOOL_OUTPUT;
        case NODE_GATE_AND:
            return APP_TOOL_AND;
        case NODE_GATE_OR:
            return APP_TOOL_OR;
        case NODE_GATE_NOT:
            return APP_TOOL_NOT;
        case NODE_GATE_XOR:
            return APP_TOOL_XOR;
        case NODE_GATE_NAND:
            return APP_TOOL_SELECT;
        case NODE_GATE_NOR:
            return APP_TOOL_SELECT;
        case NODE_GATE_DFF:
            return APP_TOOL_SELECT;
        case NODE_GATE_LATCH:
            return APP_TOOL_SELECT;
        case NODE_GATE_CLOCK:
            return APP_TOOL_CLOCK;
        default:
            return APP_TOOL_SELECT;
    }
}

NodeType app_node_type_for_tool(AppTool tool) {
    switch (tool) {
        case APP_TOOL_INPUT:
            return NODE_INPUT;
        case APP_TOOL_OUTPUT:
            return NODE_OUTPUT;
        case APP_TOOL_AND:
            return NODE_GATE_AND;
        case APP_TOOL_OR:
            return NODE_GATE_OR;
        case APP_TOOL_NOT:
            return NODE_GATE_NOT;
        case APP_TOOL_XOR:
            return NODE_GATE_XOR;
        case APP_TOOL_CLOCK:
            return NODE_GATE_CLOCK;
        case APP_TOOL_SELECT:
            return NODE_INPUT;
        default:
            return NODE_INPUT;
    }
}

bool app_tool_places_node(AppTool tool) {
    return tool != APP_TOOL_SELECT;
}

const char* app_mode_label(AppMode mode) {
    switch (mode) {
        case MODE_COMPARE:
            return "Compare";
        case MODE_BUILD:
            return "Edit";
        default:
            return "Edit";
    }
}

const char* app_tool_label(AppTool tool) {
    switch (tool) {
        case APP_TOOL_INPUT:
            return "Input";
        case APP_TOOL_OUTPUT:
            return "Output";
        case APP_TOOL_AND:
            return "AND";
        case APP_TOOL_OR:
            return "OR";
        case APP_TOOL_NOT:
            return "NOT";
        case APP_TOOL_XOR:
            return "XOR";
        case APP_TOOL_CLOCK:
            return "Clock";
        case APP_TOOL_SELECT:
            return "Select";
        default:
            return "Select";
    }
}

void app_update_kmap_grouping(AppContext *app) {
    static const Color colors[] = {
        { 255, 0, 0, 100 },
        { 0, 255, 0, 100 },
        { 0, 0, 255, 100 },
        { 255, 165, 0, 100 }
    };
    static const uint8_t groups2_masks[] = { 0x5U, 0xAU, 0x3U, 0xCU };
    static const char *groups2_terms[] = { "B'", "B", "A'", "A" };
    static const char *groups1_terms[] = { "A'B'", "A'B", "AB'", "AB" };
    char buf[256] = {0};
    uint32_t cols;
    uint32_t table_bits;
    uint32_t i;
    uint8_t covered;

    app->kmap_group_count = 0;
    free(app->simplified_expression);
    app->simplified_expression = NULL;

    if (!app->current_table || app->current_table->input_count != 2) {
        return;
    }

    cols = (uint32_t)app->current_table->input_count + (uint32_t)app->current_table->output_count;
    table_bits = 0U;
    covered = 0U;
    for (i = 0; i < 4U; i++) {
        uint32_t cell_index;

        cell_index = (i * cols) + 2U;
        if (app->current_table->data[cell_index] == LOGIC_HIGH) {
            table_bits |= (1U << i);
        }
    }

    if (table_bits == 0U) {
        app->simplified_expression = strdup("0");
        return;
    }
    if (table_bits == 0xFU) {
        KMapGroup *group;

        group = &app->kmap_groups[app->kmap_group_count++];
        group->cell_mask = 0xFU;
        strcpy(group->term, "1");
        group->color = colors[0];
        app->simplified_expression = strdup("1");
        return;
    }

    for (i = 0; i < 4U; i++) {
        uint32_t uncovered_mask;

        if ((table_bits & groups2_masks[i]) != groups2_masks[i]) {
            continue;
        }

        uncovered_mask = table_bits & (uint32_t)groups2_masks[i] & (uint32_t)(uint8_t)(~covered);
        if (uncovered_mask != 0U) {
            KMapGroup *group;

            group = &app->kmap_groups[app->kmap_group_count++];
            group->cell_mask = groups2_masks[i];
            strcpy(group->term, groups2_terms[i]);
            group->color = colors[app->kmap_group_count % 4U];
            covered = (uint8_t)(covered | groups2_masks[i]);
        }
    }

    for (i = 0; i < 4U; i++) {
        uint8_t cell_mask;

        cell_mask = (uint8_t)(1U << i);
        if ((table_bits & (uint32_t)cell_mask) == 0U || (covered & cell_mask) != 0U) {
            continue;
        }

        {
            KMapGroup *group;

            group = &app->kmap_groups[app->kmap_group_count++];
            group->cell_mask = cell_mask;
            strcpy(group->term, groups1_terms[i]);
            group->color = colors[app->kmap_group_count % 4U];
            covered = (uint8_t)(covered | cell_mask);
        }
    }

    for (i = 0; i < app->kmap_group_count; i++) {
        if (i > 0U) {
            strcat(buf, " OR ");
        }
        strcat(buf, app->kmap_groups[i].term);
    }
    app->simplified_expression = strdup(buf);
}
