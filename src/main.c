#include "raylib.h"
#include "app.h"
#include "circuit_file.h"
#include "draw_util.h"
#include "ui.h"
#include "workspace_layout.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define TAB_COUNT 2
#define TOPBAR_TAB_START_X 130.0f
#define TOPBAR_TAB_WIDTH 88.0f
#define TOPBAR_TAB_HEIGHT 28.0f
#define TOPBAR_TAB_GAP 4.0f
#define TOPBAR_BUTTON_WIDTH 80.0f
#define TOPBAR_BUTTON_HEIGHT 28.0f
#define TOPBAR_BUTTON_GAP 10.0f
#define TOPBAR_HELP_WIDTH 30.0f
#define TOPBAR_HELP_MARGIN_RIGHT 20.0f
#define TOPBAR_HELP_GAP 14.0f

typedef struct {
    char path[APP_SOURCE_PATH_MAX];
    time_t modified_time;
    off_t size;
    bool active;
    uint8_t _padding[7];
} SourceWatch;

typedef struct {
    Rectangle tabs[TAB_COUNT];
    Rectangle sim_buttons[3];
    Rectangle help_button;
} TopbarLayout;

static TopbarLayout compute_topbar_layout(const WorkspaceFrame *frame) {
    TopbarLayout layout;
    float button_start_x;
    int i;

    memset(&layout, 0, sizeof(layout));

    for (i = 0; i < TAB_COUNT; i++) {
        layout.tabs[i] = (Rectangle){
            TOPBAR_TAB_START_X + ((float)i * (TOPBAR_TAB_WIDTH + TOPBAR_TAB_GAP)),
            11.0f,
            TOPBAR_TAB_WIDTH,
            TOPBAR_TAB_HEIGHT
        };
    }

    layout.help_button = (Rectangle){
        (float)frame->window_width - TOPBAR_HELP_MARGIN_RIGHT - TOPBAR_HELP_WIDTH,
        11.0f,
        TOPBAR_HELP_WIDTH,
        TOPBAR_BUTTON_HEIGHT
    };

    button_start_x = layout.help_button.x - TOPBAR_HELP_GAP - (3.0f * TOPBAR_BUTTON_WIDTH) - (2.0f * TOPBAR_BUTTON_GAP);
    for (i = 0; i < 3; i++) {
        layout.sim_buttons[i] = (Rectangle){
            button_start_x + ((float)i * (TOPBAR_BUTTON_WIDTH + TOPBAR_BUTTON_GAP)),
            11.0f,
            TOPBAR_BUTTON_WIDTH,
            TOPBAR_BUTTON_HEIGHT
        };
    }

    return layout;
}

static int mouse_cursor_for_handle(WorkspaceResizeHandle handle) {
    if (handle == WORKSPACE_RESIZE_HANDLE_TOOLBOX || handle == WORKSPACE_RESIZE_HANDLE_SIDE_PANEL) {
        return MOUSE_CURSOR_RESIZE_EW;
    }
    if (handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL) {
        return MOUSE_CURSOR_RESIZE_NS;
    }

    return MOUSE_CURSOR_DEFAULT;
}

static void draw_resize_seam(Rectangle rect, WorkspaceResizeHandle handle) {
    Color seam_color;

    if (handle == WORKSPACE_RESIZE_HANDLE_NONE) {
        return;
    }

    seam_color = (Color){ 52, 52, 52, 200 };
    if (handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL) {
        DrawRectangle(
            pixel(rect.x),
            pixel(rect.y + (rect.height * 0.5f)),
            pixel(rect.width),
            1,
            seam_color
        );
    } else {
        DrawRectangle(
            pixel(rect.x + (rect.width * 0.5f)),
            pixel(rect.y),
            1,
            pixel(rect.height),
            seam_color
        );
    }
}

static void draw_resize_handle(Rectangle rect, WorkspaceResizeHandle handle, bool hovered, bool active) {
    Color grip_color;
    Rectangle grip;
    float grip_length;
    float grip_thickness;

    if (handle == WORKSPACE_RESIZE_HANDLE_NONE) {
        return;
    }
    if (!hovered && !active) {
        return;
    }

    grip = rect;
    if (handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL) {
        grip_length = 92.0f;
        grip_thickness = active ? 4.0f : 3.0f;
        grip.x += (rect.width - grip_length) * 0.5f;
        grip.width = grip_length;
        if (grip.width > rect.width - 24.0f) {
            grip.width = rect.width - 24.0f;
            grip.x = rect.x;
        }
        if (grip.width < 24.0f) {
            grip.width = rect.width;
            grip.x = rect.x;
        }
        grip.y += (rect.height - grip_thickness) * 0.5f;
        grip.height = grip_thickness;
    } else {
        grip_length = 92.0f;
        grip_thickness = active ? 4.0f : 3.0f;
        grip.y += (rect.height - grip_length) * 0.5f;
        grip.height = grip_length;
        if (grip.height > rect.height - 24.0f) {
            grip.height = rect.height - 24.0f;
            grip.y = rect.y;
        }
        if (grip.height < 24.0f) {
            grip.height = rect.height;
            grip.y = rect.y;
        }
        grip.x += (rect.width - grip_thickness) * 0.5f;
        grip.width = grip_thickness;
    }

    grip_color = active ? (Color){ 200, 170, 255, 220 } : (Color){ 170, 170, 170, 140 };
    DrawRectangleRounded(grip, 1.0f, 12, grip_color);
}

static bool topbar_sim_controls_visible(AppMode mode) {
    /* Editing and simulating are a single mode now; sim controls are always
       present. The only other real mode is COMPARE, which also runs live. */
    (void)mode;
    return true;
}

static AppMode topbar_mode_for_tab(int index) {
    if (index == 1) {
        return MODE_COMPARE;
    }
    return MODE_BUILD;
}

static EditorCommand topbar_command_for_tab(int index) {
    if (index == 1) {
        return EDITOR_COMMAND_MODE_COMPARE;
    }
    return EDITOR_COMMAND_MODE_BUILD;
}

static const char *topbar_tab_label(int index) {
    if (index == 1) {
        return "COMPARE";
    }
    return "EDIT";
}

static LogicNode *find_node_at(AppContext *app, Vector2 mouse_pos) {
    int i;

    for (i = (int)app->graph.node_count - 1; i >= 0; i--) {
        LogicNode *node;

        node = &app->graph.nodes[i];
        if (node->type == (NodeType)-1) {
            continue;
        }
        if (CheckCollisionPointRec(mouse_pos, node->rect)) {
            return node;
        }
    }

    return NULL;
}

static void queue_keyboard_commands(AppContext *app) {
    bool shift_down;

    shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (IsKeyPressed(KEY_V)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_SELECT);
    }
    if (IsKeyPressed(KEY_ONE)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_INPUT);
    }
    if (IsKeyPressed(KEY_TWO)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_OUTPUT);
    }
    if (IsKeyPressed(KEY_THREE)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_AND);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_OR);
    }
    if (IsKeyPressed(KEY_FIVE)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_NOT);
    }
    if (IsKeyPressed(KEY_SIX)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_XOR);
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        app_queue_command(app, EDITOR_COMMAND_TOOL_CLOCK);
    }
    if (IsKeyPressed(KEY_B)) {
        app_queue_command(app, EDITOR_COMMAND_MODE_BUILD);
    }
    if (IsKeyPressed(KEY_C)) {
        app_queue_command(app, EDITOR_COMMAND_MODE_COMPARE);
    }
    if (IsKeyPressed(KEY_R)) {
        app_queue_command(app, EDITOR_COMMAND_SIM_RESET);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        app_queue_command(app, EDITOR_COMMAND_SIM_TOGGLE);
    }
    if (IsKeyPressed(KEY_PERIOD)) {
        app_queue_command(app, EDITOR_COMMAND_SIM_STEP);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        app_queue_command(app, EDITOR_COMMAND_CANCEL);
    }
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
        app_queue_command(app, EDITOR_COMMAND_DELETE_SELECTION);
    }
    if (IsKeyPressed(KEY_TAB)) {
        app_queue_command(app, shift_down ? EDITOR_COMMAND_SELECT_PREVIOUS_NODE : EDITOR_COMMAND_SELECT_NEXT_NODE);
    }
    if (IsKeyPressed(KEY_LEFT)) {
        app_queue_command(app, EDITOR_COMMAND_MOVE_SELECTION_LEFT);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        app_queue_command(app, EDITOR_COMMAND_MOVE_SELECTION_RIGHT);
    }
    if (IsKeyPressed(KEY_UP)) {
        app_queue_command(
            app,
            (app->selected_node && app->focused_panel == APP_PANEL_CANVAS) ?
                EDITOR_COMMAND_MOVE_SELECTION_UP :
                EDITOR_COMMAND_SELECT_PREVIOUS_ROW
        );
    }
    if (IsKeyPressed(KEY_DOWN)) {
        app_queue_command(
            app,
            (app->selected_node && app->focused_panel == APP_PANEL_CANVAS) ?
                EDITOR_COMMAND_MOVE_SELECTION_DOWN :
                EDITOR_COMMAND_SELECT_NEXT_ROW
        );
    }
}

static void process_command_queue(AppContext *app) {
    EditorCommand command;

    while (app_pop_command(app, &command)) {
        app_handle_command(app, command);
    }
}

static void queue_mouse_toolbar_commands(AppContext *app, Vector2 mouse_pos, const Rectangle *toolbox_rect, const TopbarLayout *topbar_layout, bool mouse_left_pressed, bool *shortcuts_open) {
    int i;

    if (!mouse_left_pressed) {
        return;
    }

    if (CheckCollisionPointRec(mouse_pos, topbar_layout->help_button)) {
        *shortcuts_open = !(*shortcuts_open);
        return;
    }

    for (i = 0; i < TAB_COUNT; i++) {
        if (CheckCollisionPointRec(mouse_pos, topbar_layout->tabs[i])) {
            app_queue_command(app, topbar_command_for_tab(i));
            return;
        }
    }

    if (topbar_sim_controls_visible(app->mode)) {
        if (CheckCollisionPointRec(mouse_pos, topbar_layout->sim_buttons[0])) {
            app_queue_command(app, EDITOR_COMMAND_SIM_TOGGLE);
            return;
        }
        if (CheckCollisionPointRec(mouse_pos, topbar_layout->sim_buttons[1])) {
            app_queue_command(app, EDITOR_COMMAND_SIM_STEP);
            return;
        }
        if (CheckCollisionPointRec(mouse_pos, topbar_layout->sim_buttons[2])) {
            app_queue_command(app, EDITOR_COMMAND_SIM_RESET);
            return;
        }
    }

    if (CheckCollisionPointRec(mouse_pos, *toolbox_rect)) {
        static const AppTool slot_tools[] = {
            APP_TOOL_INPUT,
            APP_TOOL_OUTPUT,
            APP_TOOL_AND,
            APP_TOOL_OR,
            APP_TOOL_NOT,
            APP_TOOL_XOR,
            APP_TOOL_CLOCK,
        };
        static const EditorCommand slot_commands[] = {
            EDITOR_COMMAND_TOOL_INPUT,
            EDITOR_COMMAND_TOOL_OUTPUT,
            EDITOR_COMMAND_TOOL_AND,
            EDITOR_COMMAND_TOOL_OR,
            EDITOR_COMMAND_TOOL_NOT,
            EDITOR_COMMAND_TOOL_XOR,
            EDITOR_COMMAND_TOOL_CLOCK,
        };
        int slot;

        slot = ui_toolbox_slot_at(*toolbox_rect, mouse_pos);
        if (slot >= 0 && slot < (int)(sizeof(slot_tools) / sizeof(slot_tools[0]))) {
            if (app->active_tool == slot_tools[slot]) {
                app_queue_command(app, EDITOR_COMMAND_TOOL_SELECT);
            } else {
                app_queue_command(app, slot_commands[slot]);
            }
        }
    }
}

static bool point_is_toolbar(Vector2 mouse_pos, const WorkspaceFrame *frame) {
    return CheckCollisionPointRec(mouse_pos, frame->topbar_rect) || CheckCollisionPointRec(mouse_pos, frame->toolbox_rect);
}

static bool select_truth_row_at(AppContext *app, Rectangle side_panel_rect, Vector2 mouse_pos) {
    UiContextPanelLayout layout;
    uint32_t row_index;

    if (!CheckCollisionPointRec(mouse_pos, side_panel_rect) || !app->current_table) {
        return false;
    }

    layout = ui_measure_context_panel(app, side_panel_rect);
    for (row_index = 0U; row_index < layout.visible_truth_rows; row_index++) {
        Rectangle row_rect;

        if (!ui_context_truth_table_row_rect(app, &layout, row_index, &row_rect)) {
            continue;
        }
        if (CheckCollisionPointRec(mouse_pos, row_rect)) {
            app_select_row(app, row_index);
            return true;
        }
    }

    return false;
}

static bool source_watch_refresh(SourceWatch *watch, bool *changed) {
    struct stat info;

    if (!watch || !watch->active || watch->path[0] == '\0') {
        return false;
    }
    if (stat(watch->path, &info) != 0) {
        return false;
    }

    *changed = (info.st_mtime != watch->modified_time) || (info.st_size != watch->size);
    watch->modified_time = info.st_mtime;
    watch->size = info.st_size;
    return true;
}

static bool load_circuit_into_app(AppContext *app, const char *path, const char *status_prefix) {
    char error_message[APP_STATUS_MESSAGE_MAX];
    char status_message[APP_STATUS_MESSAGE_MAX];

    if (!circuit_file_load(app, path, error_message, sizeof(error_message))) {
        snprintf(status_message, sizeof(status_message), "Load failed: %s", error_message);
        app_set_source_status(app, status_message);
        return false;
    }

    app_set_source_path(app, path);
    snprintf(status_message, sizeof(status_message), "%s", status_prefix);
    app_set_source_status(app, status_message);
    return true;
}

static const char *parse_load_path(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--load") == 0 && i + 1 < argc) {
            return argv[i + 1];
        }
        if (argv[i][0] != '-') {
            return argv[i];
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    AppContext app;
    WorkspaceLayoutPrefs layout_prefs;
    SourceWatch source_watch;
    const char *load_path;
    Vector2 click_start_pos;
    Vector2 pan_last_mouse;
    WorkspaceResizeHandle active_resize_handle;
    bool canvas_pan_active;
    bool canvas_pan_moved;
    bool click_moved;
    bool layout_save_pending;
    bool shortcuts_open;

    click_start_pos = (Vector2){ 0.0f, 0.0f };
    pan_last_mouse = (Vector2){ 0.0f, 0.0f };
    active_resize_handle = WORKSPACE_RESIZE_HANDLE_NONE;
    canvas_pan_active = false;
    canvas_pan_moved = false;
    click_moved = false;
    layout_save_pending = false;
    shortcuts_open = false;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WORKSPACE_WINDOW_START_WIDTH, WORKSPACE_WINDOW_START_HEIGHT, "Multi-View Logic Debugger");
    SetWindowMinSize(WORKSPACE_WINDOW_MIN_WIDTH, WORKSPACE_WINDOW_MIN_HEIGHT);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    memset(&source_watch, 0, sizeof(source_watch));
    workspace_layout_init_defaults(&layout_prefs);
    workspace_layout_load_prefs(&layout_prefs);
    app_init(&app);
    app.target_graph = NULL;
    app_update_logic(&app);

    load_path = parse_load_path(argc, argv);
    if (load_path) {
        if (load_circuit_into_app(&app, load_path, "Loaded from file")) {
            bool changed;

            snprintf(source_watch.path, sizeof(source_watch.path), "%s", load_path);
            source_watch.active = true;
            app.source_live_reload = true;
            changed = false;
            source_watch_refresh(&source_watch, &changed);
        }
    }

    while (!WindowShouldClose()) {
        WorkspaceFrame frame_layout;
        WorkspaceResizeHandles resize_handles;
        TopbarLayout topbar_layout;
        WorkspaceResizeHandle hovered_resize_handle;
        Vector2 mouse_pos;
        Vector2 world_mouse_pos;
        Vector2 zoom_anchor;
        bool canvas_hovered;
        bool canvas_pan_blocking_interactions;
        bool mouse_left_pressed;
        bool mouse_left_down;
        bool mouse_left_released;
        bool mouse_right_pressed;

        workspace_layout_sanitize_prefs(&layout_prefs, GetScreenWidth(), GetScreenHeight());
        frame_layout = workspace_layout_compute_frame(&layout_prefs, GetScreenWidth(), GetScreenHeight());
        resize_handles = workspace_layout_compute_handles(&frame_layout);
        topbar_layout = compute_topbar_layout(&frame_layout);

        mouse_pos = GetMousePosition();
        hovered_resize_handle = workspace_layout_hit_test_handle(&resize_handles, mouse_pos);
        canvas_hovered = CheckCollisionPointRec(mouse_pos, frame_layout.canvas_rect);
        world_mouse_pos = app_canvas_screen_to_world(&app, frame_layout.canvas_rect, mouse_pos);
        zoom_anchor = canvas_hovered ?
            mouse_pos :
            (Vector2){
                frame_layout.canvas_rect.x + (frame_layout.canvas_rect.width * 0.5f),
                frame_layout.canvas_rect.y + (frame_layout.canvas_rect.height * 0.5f)
            };
        canvas_pan_blocking_interactions = false;
        mouse_left_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        mouse_left_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        mouse_left_released = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        mouse_right_pressed = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);

        if (active_resize_handle != WORKSPACE_RESIZE_HANDLE_NONE) {
            SetMouseCursor(mouse_cursor_for_handle(active_resize_handle));
        } else if (canvas_pan_active) {
            SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
        } else {
            SetMouseCursor(mouse_cursor_for_handle(hovered_resize_handle));
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            mouse_left_pressed &&
            hovered_resize_handle != WORKSPACE_RESIZE_HANDLE_NONE) {
            active_resize_handle = hovered_resize_handle;
            app.drag_node = NULL;
            app_cancel_wire_drag(&app);
        }

        if (!shortcuts_open) {
            queue_keyboard_commands(&app);
        }
        if (IsKeyPressed(KEY_SLASH)) {
            shortcuts_open = !shortcuts_open;
        } else if (shortcuts_open && IsKeyPressed(KEY_ESCAPE)) {
            shortcuts_open = false;
        }
        if (!shortcuts_open && IsKeyPressed(KEY_HOME)) {
            app_reset_canvas_view(&app);
        }
        if (!shortcuts_open &&
            (canvas_hovered || app.focused_panel == APP_PANEL_CANVAS) &&
            (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD))) {
            app_zoom_canvas_at(&app, frame_layout.canvas_rect, zoom_anchor, 1.12f);
        }
        if (!shortcuts_open &&
            (canvas_hovered || app.focused_panel == APP_PANEL_CANVAS) &&
            (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT))) {
            app_zoom_canvas_at(&app, frame_layout.canvas_rect, zoom_anchor, 1.0f / 1.12f);
        }
        if (!(mouse_left_pressed && hovered_resize_handle != WORKSPACE_RESIZE_HANDLE_NONE)) {
            queue_mouse_toolbar_commands(&app, mouse_pos, &frame_layout.toolbox_rect, &topbar_layout, mouse_left_pressed, &shortcuts_open);
        }
        process_command_queue(&app);

        if (active_resize_handle != WORKSPACE_RESIZE_HANDLE_NONE) {
            if (mouse_left_down &&
                workspace_layout_apply_drag(
                    &layout_prefs,
                    active_resize_handle,
                    mouse_pos,
                    frame_layout.window_width,
                    frame_layout.window_height
                )) {
                layout_save_pending = true;
                frame_layout = workspace_layout_compute_frame(&layout_prefs, frame_layout.window_width, frame_layout.window_height);
                resize_handles = workspace_layout_compute_handles(&frame_layout);
                topbar_layout = compute_topbar_layout(&frame_layout);
            }
            if (mouse_left_released) {
                active_resize_handle = WORKSPACE_RESIZE_HANDLE_NONE;
                if (layout_save_pending) {
                    workspace_layout_save_prefs(&layout_prefs);
                    layout_save_pending = false;
                }
            }
        }

        if (source_watch.active) {
            bool changed;

            changed = false;
            if (source_watch_refresh(&source_watch, &changed) && changed) {
                load_circuit_into_app(&app, source_watch.path, "Reloaded from file");
            }
        }

        if (canvas_pan_active) {
            canvas_pan_blocking_interactions = true;
            if (mouse_left_down) {
                Vector2 pan_delta;
                float dx;
                float dy;

                pan_delta = (Vector2){
                    mouse_pos.x - pan_last_mouse.x,
                    mouse_pos.y - pan_last_mouse.y
                };
                if (pan_delta.x != 0.0f || pan_delta.y != 0.0f) {
                    dx = mouse_pos.x - click_start_pos.x;
                    dy = mouse_pos.y - click_start_pos.y;
                    if (!canvas_pan_moved && ((dx * dx) + (dy * dy) > 16.0f)) {
                        canvas_pan_moved = true;
                    }
                    app_pan_canvas(&app, pan_delta);
                    pan_last_mouse = mouse_pos;
                }
            } else {
                canvas_pan_active = false;
            }
        }

        world_mouse_pos = app_canvas_screen_to_world(&app, frame_layout.canvas_rect, mouse_pos);

        if (app.wire_drag_active) {
            app_update_wire_drag(
                &app,
                ui_get_pin_at(&app, frame_layout.canvas_rect, mouse_pos),
                world_mouse_pos
            );
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            !canvas_pan_blocking_interactions &&
            mouse_left_pressed &&
            !point_is_toolbar(mouse_pos, &frame_layout) &&
            !app_tool_places_node(app.active_tool)) {
            LogicPin *pin;

            pin = ui_get_pin_at(&app, frame_layout.canvas_rect, mouse_pos);
            if (pin && CheckCollisionPointRec(mouse_pos, frame_layout.canvas_rect)) {
                app_begin_wire_drag(&app, pin, world_mouse_pos);
            } else if (CheckCollisionPointRec(mouse_pos, frame_layout.canvas_rect)) {
                LogicPin *wire_sink;
                LogicNode *hit_node;

                app.drag_node = NULL;
                app_set_panel_focus(&app, APP_PANEL_CANVAS);
                wire_sink = ui_get_wire_at(&app, frame_layout.canvas_rect, mouse_pos);
                hit_node = find_node_at(&app, world_mouse_pos);
                if (wire_sink) {
                    app_cancel_wire_drag(&app);
                    app_select_wire_by_sink(&app, wire_sink);
                } else if (hit_node) {
                    app_cancel_wire_drag(&app);
                    app.drag_node = hit_node;
                    app.selected_node = hit_node;
                    app.selected_wire_sink = NULL;
                    app.drag_offset = (Vector2){
                        world_mouse_pos.x - hit_node->pos.x,
                        world_mouse_pos.y - hit_node->pos.y
                    };
                    click_start_pos = mouse_pos;
                    click_moved = false;
                } else {
                    app_cancel_wire_drag(&app);
                    canvas_pan_active = true;
                    canvas_pan_moved = false;
                    pan_last_mouse = mouse_pos;
                    click_start_pos = mouse_pos;
                }
            } else if (select_truth_row_at(&app, frame_layout.side_panel_rect, mouse_pos)) {
                /* Selection handled by the shared truth-table layout helpers. */
            }
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            !canvas_pan_blocking_interactions &&
            mouse_left_released &&
            app_tool_places_node(app.active_tool) &&
            CheckCollisionPointRec(mouse_pos, frame_layout.canvas_rect)) {
            LogicNode *new_node;

            new_node = app_add_node(
                &app,
                app_node_type_for_tool(app.active_tool),
                app_snap_node_position(world_mouse_pos, app_node_type_for_tool(app.active_tool))
            );
            if (new_node) {
                app.selected_node = new_node;
                app_set_panel_focus(&app, APP_PANEL_CANVAS);
                app_update_logic(&app);
            }
            app_set_tool(&app, APP_TOOL_SELECT);
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            !canvas_pan_blocking_interactions &&
            mouse_left_released &&
            app.wire_drag_active) {
            app_commit_wire_drag(&app, ui_get_pin_at(&app, frame_layout.canvas_rect, mouse_pos));
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            !canvas_pan_blocking_interactions &&
            mouse_left_down &&
            app.drag_node) {
            float dx = mouse_pos.x - click_start_pos.x;
            float dy = mouse_pos.y - click_start_pos.y;
            if (!click_moved && ((dx * dx) + (dy * dy) > 16.0f)) {
                click_moved = true;
            }
            if (click_moved) {
                app.drag_node->pos.x = world_mouse_pos.x - app.drag_offset.x;
                app.drag_node->pos.y = world_mouse_pos.y - app.drag_offset.y;
                app.drag_node->pos = app_snap_node_position(app.drag_node->pos, app.drag_node->type);
                app.drag_node->rect.x = app.drag_node->pos.x;
                app.drag_node->rect.y = app.drag_node->pos.y;
            }
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            !canvas_pan_blocking_interactions &&
            mouse_left_released) {
            if (app.drag_node && !click_moved && app.drag_node->type == NODE_INPUT) {
                app_toggle_input_value(&app, app.drag_node);
            }
            app.drag_node = NULL;
            click_moved = false;
        }

        if (active_resize_handle == WORKSPACE_RESIZE_HANDLE_NONE &&
            mouse_left_released &&
            !canvas_pan_active &&
            CheckCollisionPointRec(mouse_pos, frame_layout.canvas_rect) &&
            !app_tool_places_node(app.active_tool) &&
            !app.drag_node &&
            !app.wire_drag_active &&
            ui_get_pin_at(&app, frame_layout.canvas_rect, mouse_pos) == NULL &&
            ui_get_wire_at(&app, frame_layout.canvas_rect, mouse_pos) == NULL &&
            find_node_at(&app, world_mouse_pos) == NULL &&
            !canvas_pan_moved) {
            app.selected_node = NULL;
            app.selected_wire_sink = NULL;
        }

        if (mouse_left_released && !canvas_pan_active) {
            canvas_pan_moved = false;
        }

        if (mouse_right_pressed) {
            canvas_pan_active = false;
            canvas_pan_moved = false;
            app_cancel_interaction(&app);
        }

        app_update_simulation(&app);
        app_compute_view_context(&app);

        BeginDrawing();
        ClearBackground((Color){ 24, 24, 24, 255 });

        DrawRectangle(0, 0, frame_layout.window_width, pixel(WORKSPACE_TOPBAR_HEIGHT), (Color){ 20, 20, 20, 255 });
        draw_line_at(
            0.0f,
            WORKSPACE_TOPBAR_HEIGHT,
            (float)frame_layout.window_width,
            WORKSPACE_TOPBAR_HEIGHT,
            (Color){ 50, 50, 50, 255 }
        );
        draw_text_at("MVLD", 20.0f, 16.0f, 20, LIGHTGRAY);

        {
            Color violet;
            Color dim_label;
            Color active_label;
            Color idle_fill;
            int i;

            violet = (Color){ 200, 170, 255, 255 };
            dim_label = (Color){ 150, 150, 150, 255 };
            active_label = (Color){ 240, 240, 240, 255 };
            idle_fill = (Color){ 46, 46, 46, 255 };

            for (i = 0; i < TAB_COUNT; i++) {
                Rectangle tab;
                bool is_active;
                bool is_hover;
                const char *label;
                float label_x;

                tab = topbar_layout.tabs[i];
                is_active = (topbar_mode_for_tab(i) == app.mode);
                is_hover = CheckCollisionPointRec(mouse_pos, tab);
                label = topbar_tab_label(i);

                if (is_hover && !is_active) {
                    DrawRectangleRec(tab, (Color){ 30, 30, 30, 255 });
                }
                label_x = tab.x + (tab.width - text_width(label, 13)) * 0.5f;
                draw_text_at(label, label_x, tab.y + 8.0f, 13, is_active ? active_label : dim_label);
                if (is_active) {
                    DrawRectangle(pixel(tab.x + 8.0f), pixel(tab.y + tab.height), pixel(tab.width - 16.0f), 2, violet);
                }
            }

            if (topbar_sim_controls_visible(app.mode)) {
                Rectangle run_rect;
                Rectangle step_rect;
                Rectangle reset_rect;
                const char *run_label;
                Color run_color;
                float run_label_x;
                float step_label_x;
                float reset_label_x;

                run_rect = topbar_layout.sim_buttons[0];
                step_rect = topbar_layout.sim_buttons[1];
                reset_rect = topbar_layout.sim_buttons[2];

                run_label = app.sim_active ? "STOP" : "RUN";
                run_color = app.sim_active ? (Color){ 200, 60, 60, 255 } : (Color){ 245, 185, 50, 255 };

                DrawRectangleRounded(run_rect, 0.24f, 8, run_color);
                DrawRectangleRounded(step_rect, 0.24f, 8, idle_fill);
                DrawRectangleRounded(reset_rect, 0.24f, 8, idle_fill);

                run_label_x = run_rect.x + (run_rect.width - text_width(run_label, 13)) * 0.5f;
                step_label_x = step_rect.x + (step_rect.width - text_width("STEP", 13)) * 0.5f;
                reset_label_x = reset_rect.x + (reset_rect.width - text_width("RESET", 13)) * 0.5f;

                draw_text_at(run_label, run_label_x, run_rect.y + 8.0f, 13, WHITE);
                draw_text_at("STEP", step_label_x, step_rect.y + 8.0f, 13, LIGHTGRAY);
                draw_text_at("RESET", reset_label_x, reset_rect.y + 8.0f, 13, LIGHTGRAY);
            }

            {
                Rectangle help_rect;
                bool help_hover;

                help_rect = topbar_layout.help_button;
                help_hover = CheckCollisionPointRec(mouse_pos, help_rect);
                if (shortcuts_open || help_hover) {
                    DrawRectangleRounded(help_rect, 0.5f, 8, (Color){ 38, 38, 38, 255 });
                }
                DrawRectangleRoundedLinesEx(help_rect, 0.5f, 8, 1.0f, (Color){ 70, 70, 70, 255 });
                draw_text_at("?", help_rect.x + (help_rect.width - text_width("?", 14)) * 0.5f, help_rect.y + 7.0f, 14, LIGHTGRAY);
            }
        }

        if (begin_scissor_rect(frame_layout.canvas_rect)) {
            ui_draw_circuit(&app, frame_layout.canvas_rect);
            ui_draw_placement_ghost(&app, frame_layout.canvas_rect, mouse_pos);
            EndScissorMode();
        }
        ui_draw_toolbox(&app, frame_layout.toolbox_rect);
        if (begin_scissor_rect(frame_layout.wave_rect)) {
            ui_draw_waveforms(&app, frame_layout.wave_rect);
            EndScissorMode();
        }

        DrawRectangleRec(frame_layout.side_panel_rect, (Color){ 23, 23, 23, 255 });

        if (begin_scissor_rect(frame_layout.side_panel_rect)) {
            ui_draw_context_panel(&app, frame_layout.side_panel_rect);
            EndScissorMode();
        }

        draw_resize_seam(resize_handles.toolbox, WORKSPACE_RESIZE_HANDLE_TOOLBOX);
        draw_resize_seam(resize_handles.side_panel, WORKSPACE_RESIZE_HANDLE_SIDE_PANEL);
        draw_resize_seam(resize_handles.wave_panel, WORKSPACE_RESIZE_HANDLE_WAVE_PANEL);

        draw_resize_handle(
            resize_handles.toolbox,
            WORKSPACE_RESIZE_HANDLE_TOOLBOX,
            hovered_resize_handle == WORKSPACE_RESIZE_HANDLE_TOOLBOX,
            active_resize_handle == WORKSPACE_RESIZE_HANDLE_TOOLBOX
        );
        draw_resize_handle(
            resize_handles.side_panel,
            WORKSPACE_RESIZE_HANDLE_SIDE_PANEL,
            hovered_resize_handle == WORKSPACE_RESIZE_HANDLE_SIDE_PANEL,
            active_resize_handle == WORKSPACE_RESIZE_HANDLE_SIDE_PANEL
        );
        draw_resize_handle(
            resize_handles.wave_panel,
            WORKSPACE_RESIZE_HANDLE_WAVE_PANEL,
            hovered_resize_handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL,
            active_resize_handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL
        );

        {
            char footer_text[APP_SOURCE_PATH_MAX + 96];
            const char *file_label;
            uint32_t live_count;
            uint32_t i;
            char sim_segment[48];
            char selected_label[64];
            char selected_segment[80];

            DrawRectangleRec(frame_layout.footer_rect, (Color){ 20, 20, 20, 255 });
            draw_line_at(
                frame_layout.footer_rect.x,
                frame_layout.footer_rect.y,
                frame_layout.footer_rect.x + frame_layout.footer_rect.width,
                frame_layout.footer_rect.y,
                (Color){ 50, 50, 50, 255 }
            );

            file_label = app.source_path[0] ? app.source_path : "untitled.circ";
            live_count = 0U;
            for (i = 0; i < app.graph.node_count; i++) {
                if (app.graph.nodes[i].type != (NodeType)-1) {
                    live_count++;
                }
            }

            sim_segment[0] = '\0';
            if (topbar_sim_controls_visible(app.mode)) {
                snprintf(sim_segment, sizeof(sim_segment), "  |  sim @ %.0f Hz", (double)app.sim_speed);
            }

            selected_label[0] = '\0';
            if (app.selected_wire_sink && app.selected_wire_sink->node &&
                app.selected_wire_sink->node->type != (NodeType)-1) {
                snprintf(
                    selected_label,
                    sizeof(selected_label),
                    "wire -> %s.in%u",
                    app.selected_wire_sink->node->name ? app.selected_wire_sink->node->name : "node",
                    app.selected_wire_sink->index
                );
            } else if (app.selected_node && app.selected_node->type != (NodeType)-1 && app.selected_node->name) {
                snprintf(selected_label, sizeof(selected_label), "%s", app.selected_node->name);
            }
            selected_segment[0] = '\0';
            if (selected_label[0] != '\0') {
                snprintf(selected_segment, sizeof(selected_segment), "  |  selected %s", selected_label);
            }

            snprintf(footer_text, sizeof(footer_text), "FILE: %s  |  %u gates%s%s", file_label, live_count, sim_segment, selected_segment);
            draw_text_at(
                footer_text,
                frame_layout.footer_rect.x + 12.0f,
                frame_layout.footer_rect.y + 6.0f,
                11,
                (Color){ 150, 150, 150, 255 }
            );
        }

        if (shortcuts_open) {
            Rectangle card;
            float card_w;
            float card_h;
            const char *lines[] = {
                "KEYBOARD SHORTCUTS",
                "",
                "V          Select tool",
                "1-7        Pick a tool (input, gate, ...)",
                "B / C      Mode: Edit / Compare",
                "Space      Run / Stop simulation",
                "Drag empty canvas  Pan canvas",
                ".          Step one tick",
                "R          Reset simulation",
                "Tab        Cycle selection (Shift for reverse)",
                "Arrows     Move selected node on grid",
                "+ / -      Zoom canvas",
                "Home       Reset canvas view",
                "Del        Delete selected node or wire",
                "Esc        Cancel or close this panel",
                "/          Toggle this panel",
            };
            int line_count;
            int line_idx;

            DrawRectangle(0, 0, frame_layout.window_width, frame_layout.window_height, (Color){ 0, 0, 0, 160 });

            card_w = 520.0f;
            card_h = 350.0f;
            card = (Rectangle){
                ((float)frame_layout.window_width - card_w) * 0.5f,
                ((float)frame_layout.window_height - card_h) * 0.5f,
                card_w,
                card_h,
            };
            DrawRectangleRounded(card, 0.04f, 8, (Color){ 30, 30, 30, 255 });
            DrawRectangleRoundedLinesEx(card, 0.04f, 8, 1.0f, (Color){ 80, 80, 80, 255 });

            line_count = (int)(sizeof(lines) / sizeof(lines[0]));
            for (line_idx = 0; line_idx < line_count; line_idx++) {
                Color color;

                color = (line_idx == 0) ? (Color){ 240, 240, 240, 255 } : (Color){ 180, 180, 180, 255 };
                draw_text_at(lines[line_idx], card.x + 22.0f, card.y + 22.0f + ((float)line_idx * 20.0f), line_idx == 0 ? 13 : 12, color);
            }

            if (mouse_left_pressed && !CheckCollisionPointRec(mouse_pos, card)) {
                shortcuts_open = false;
            }
        }

        EndDrawing();
    }

    workspace_layout_save_prefs(&layout_prefs);
    CloseWindow();
    return 0;
}
