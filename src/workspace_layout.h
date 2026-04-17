#ifndef WORKSPACE_LAYOUT_H
#define WORKSPACE_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>
#include "raylib.h"

#define WORKSPACE_WINDOW_START_WIDTH 1440
#define WORKSPACE_WINDOW_START_HEIGHT 900
#define WORKSPACE_WINDOW_MIN_WIDTH 1100
#define WORKSPACE_WINDOW_MIN_HEIGHT 720
#define WORKSPACE_TOPBAR_HEIGHT 50.0f
#define WORKSPACE_FOOTER_HEIGHT 24.0f
#define WORKSPACE_TOOLBOX_DEFAULT_WIDTH 132.0f
#define WORKSPACE_TOOLBOX_MIN_WIDTH 128.0f
#define WORKSPACE_TOOLBOX_MAX_WIDTH 208.0f
#define WORKSPACE_SIDE_PANEL_DEFAULT_WIDTH 360.0f
#define WORKSPACE_SIDE_PANEL_MIN_WIDTH 280.0f
#define WORKSPACE_SIDE_PANEL_MAX_WIDTH 520.0f
#define WORKSPACE_WAVE_DEFAULT_HEIGHT 190.0f
#define WORKSPACE_WAVE_MIN_HEIGHT 120.0f
#define WORKSPACE_WAVE_MAX_HEIGHT 420.0f
#define WORKSPACE_CENTER_MIN_WIDTH 360.0f
#define WORKSPACE_CANVAS_MIN_HEIGHT 180.0f
#define WORKSPACE_RESIZE_HANDLE_THICKNESS 10.0f

typedef enum {
    WORKSPACE_RESIZE_HANDLE_NONE,
    WORKSPACE_RESIZE_HANDLE_TOOLBOX,
    WORKSPACE_RESIZE_HANDLE_SIDE_PANEL,
    WORKSPACE_RESIZE_HANDLE_WAVE_PANEL
} WorkspaceResizeHandle;

typedef struct {
    float toolbox_width;
    float side_panel_width;
    float wave_height;
    uint8_t _padding[4];
} WorkspaceLayoutPrefs;

typedef struct {
    int window_width;
    int window_height;
    Rectangle topbar_rect;
    Rectangle side_panel_rect;
    Rectangle canvas_rect;
    Rectangle wave_rect;
    Rectangle toolbox_rect;
    Rectangle footer_rect;
} WorkspaceFrame;

typedef struct {
    Rectangle toolbox;
    Rectangle side_panel;
    Rectangle wave_panel;
} WorkspaceResizeHandles;

void workspace_layout_init_defaults(WorkspaceLayoutPrefs *prefs);
void workspace_layout_sanitize_prefs(WorkspaceLayoutPrefs *prefs, int window_width, int window_height);
WorkspaceFrame workspace_layout_compute_frame(const WorkspaceLayoutPrefs *prefs, int window_width, int window_height);
WorkspaceResizeHandles workspace_layout_compute_handles(const WorkspaceFrame *frame);
WorkspaceResizeHandle workspace_layout_hit_test_handle(const WorkspaceResizeHandles *handles, Vector2 mouse_pos);
bool workspace_layout_apply_drag(WorkspaceLayoutPrefs *prefs, WorkspaceResizeHandle handle, Vector2 mouse_pos, int window_width, int window_height);
bool workspace_layout_load_prefs(WorkspaceLayoutPrefs *prefs);
bool workspace_layout_save_prefs(const WorkspaceLayoutPrefs *prefs);

#endif // WORKSPACE_LAYOUT_H
