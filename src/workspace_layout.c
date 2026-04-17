#include "workspace_layout.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define LAYOUT_PATH_MAX 1024

static float clampf_value(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }

    return value;
}

static bool float_changed(float current, float previous) {
    float delta;

    delta = current - previous;
    if (delta < 0.0f) {
        delta = -delta;
    }

    return delta > 0.001f;
}

static float workspace_body_height(int window_height) {
    float body_height;

    body_height = (float)window_height - WORKSPACE_TOPBAR_HEIGHT - WORKSPACE_FOOTER_HEIGHT;
    if (body_height < 0.0f) {
        body_height = 0.0f;
    }

    return body_height;
}

static float workspace_toolbox_max_width(int window_width) {
    float max_width;

    max_width = (float)window_width - WORKSPACE_SIDE_PANEL_MIN_WIDTH - WORKSPACE_CENTER_MIN_WIDTH;
    if (max_width < WORKSPACE_TOOLBOX_MIN_WIDTH) {
        max_width = WORKSPACE_TOOLBOX_MIN_WIDTH;
    }
    if (max_width > WORKSPACE_TOOLBOX_MAX_WIDTH) {
        max_width = WORKSPACE_TOOLBOX_MAX_WIDTH;
    }

    return max_width;
}

static float workspace_side_panel_max_width(int window_width, float toolbox_width) {
    float max_width;

    max_width = (float)window_width - toolbox_width - WORKSPACE_CENTER_MIN_WIDTH;
    if (max_width < WORKSPACE_SIDE_PANEL_MIN_WIDTH) {
        max_width = WORKSPACE_SIDE_PANEL_MIN_WIDTH;
    }
    if (max_width > WORKSPACE_SIDE_PANEL_MAX_WIDTH) {
        max_width = WORKSPACE_SIDE_PANEL_MAX_WIDTH;
    }

    return max_width;
}

static float workspace_wave_max_height(int window_height) {
    float max_height;

    max_height = workspace_body_height(window_height) - WORKSPACE_CANVAS_MIN_HEIGHT;
    if (max_height < WORKSPACE_WAVE_MIN_HEIGHT) {
        max_height = WORKSPACE_WAVE_MIN_HEIGHT;
    }
    if (max_height > WORKSPACE_WAVE_MAX_HEIGHT) {
        max_height = WORKSPACE_WAVE_MAX_HEIGHT;
    }

    return max_height;
}

static bool workspace_layout_path(char *buffer, size_t buffer_size) {
    const char *override_path;
    const char *home;

    if (!buffer || buffer_size == 0U) {
        return false;
    }

    override_path = getenv("MLVD_LAYOUT_PATH");
    if (override_path && override_path[0] != '\0') {
        snprintf(buffer, buffer_size, "%s", override_path);
        return true;
    }

    home = getenv("HOME");
    if (home && home[0] != '\0') {
        snprintf(buffer, buffer_size, "%s/Library/Application Support/mlvd/layout.cfg", home);
        return true;
    }

    snprintf(buffer, buffer_size, ".mlvd-layout.cfg");
    return true;
}

static bool ensure_directory(const char *path) {
    char partial[LAYOUT_PATH_MAX];
    size_t i;
    size_t length;

    if (!path || path[0] == '\0') {
        return false;
    }

    snprintf(partial, sizeof(partial), "%s", path);
    length = strlen(partial);
    for (i = 1U; i < length; i++) {
        if (partial[i] != '/') {
            continue;
        }

        partial[i] = '\0';
        if (mkdir(partial, 0755) != 0 && errno != EEXIST) {
            return false;
        }
        partial[i] = '/';
    }

    if (mkdir(partial, 0755) != 0 && errno != EEXIST) {
        return false;
    }

    return true;
}

static bool ensure_layout_parent_dir(const char *path) {
    char directory[LAYOUT_PATH_MAX];
    char *slash;

    if (!path || path[0] == '\0') {
        return false;
    }

    snprintf(directory, sizeof(directory), "%s", path);
    slash = strrchr(directory, '/');
    if (!slash) {
        return true;
    }
    if (slash == directory) {
        return true;
    }

    *slash = '\0';
    return ensure_directory(directory);
}

void workspace_layout_init_defaults(WorkspaceLayoutPrefs *prefs) {
    if (!prefs) {
        return;
    }

    memset(prefs, 0, sizeof(*prefs));
    prefs->toolbox_width = WORKSPACE_TOOLBOX_DEFAULT_WIDTH;
    prefs->side_panel_width = WORKSPACE_SIDE_PANEL_DEFAULT_WIDTH;
    prefs->wave_height = WORKSPACE_WAVE_DEFAULT_HEIGHT;
}

void workspace_layout_sanitize_prefs(WorkspaceLayoutPrefs *prefs, int window_width, int window_height) {
    float toolbox_max_width;
    float side_panel_max_width;
    float wave_max_height;

    if (!prefs) {
        return;
    }

    toolbox_max_width = workspace_toolbox_max_width(window_width);
    prefs->toolbox_width = clampf_value(prefs->toolbox_width, WORKSPACE_TOOLBOX_MIN_WIDTH, toolbox_max_width);

    side_panel_max_width = workspace_side_panel_max_width(window_width, prefs->toolbox_width);
    prefs->side_panel_width = clampf_value(
        prefs->side_panel_width,
        WORKSPACE_SIDE_PANEL_MIN_WIDTH,
        side_panel_max_width
    );

    wave_max_height = workspace_wave_max_height(window_height);
    prefs->wave_height = clampf_value(prefs->wave_height, WORKSPACE_WAVE_MIN_HEIGHT, wave_max_height);
}

WorkspaceFrame workspace_layout_compute_frame(const WorkspaceLayoutPrefs *prefs, int window_width, int window_height) {
    WorkspaceFrame frame;
    WorkspaceLayoutPrefs clamped_prefs;
    float body_height;
    float center_width;
    float canvas_height;

    if (prefs) {
        clamped_prefs = *prefs;
    } else {
        workspace_layout_init_defaults(&clamped_prefs);
    }
    workspace_layout_sanitize_prefs(&clamped_prefs, window_width, window_height);

    body_height = workspace_body_height(window_height);
    center_width = (float)window_width - clamped_prefs.toolbox_width - clamped_prefs.side_panel_width;
    if (center_width < 0.0f) {
        center_width = 0.0f;
    }

    canvas_height = body_height - clamped_prefs.wave_height;
    if (canvas_height < 0.0f) {
        canvas_height = 0.0f;
    }

    memset(&frame, 0, sizeof(frame));
    frame.window_width = window_width;
    frame.window_height = window_height;
    frame.topbar_rect = (Rectangle){ 0.0f, 0.0f, (float)window_width, WORKSPACE_TOPBAR_HEIGHT };
    frame.side_panel_rect = (Rectangle){
        (float)window_width - clamped_prefs.side_panel_width,
        WORKSPACE_TOPBAR_HEIGHT,
        clamped_prefs.side_panel_width,
        body_height
    };
    frame.canvas_rect = (Rectangle){
        clamped_prefs.toolbox_width,
        WORKSPACE_TOPBAR_HEIGHT,
        center_width,
        canvas_height
    };
    frame.wave_rect = (Rectangle){
        clamped_prefs.toolbox_width,
        WORKSPACE_TOPBAR_HEIGHT + canvas_height,
        center_width,
        clamped_prefs.wave_height
    };
    frame.toolbox_rect = (Rectangle){ 0.0f, WORKSPACE_TOPBAR_HEIGHT, clamped_prefs.toolbox_width, body_height };
    frame.footer_rect = (Rectangle){
        0.0f,
        (float)window_height - WORKSPACE_FOOTER_HEIGHT,
        (float)window_width,
        WORKSPACE_FOOTER_HEIGHT
    };
    return frame;
}

WorkspaceResizeHandles workspace_layout_compute_handles(const WorkspaceFrame *frame) {
    WorkspaceResizeHandles handles;
    float half_thickness;

    memset(&handles, 0, sizeof(handles));
    if (!frame) {
        return handles;
    }

    half_thickness = WORKSPACE_RESIZE_HANDLE_THICKNESS * 0.5f;
    handles.toolbox = (Rectangle){
        frame->toolbox_rect.x + frame->toolbox_rect.width - half_thickness,
        frame->toolbox_rect.y,
        WORKSPACE_RESIZE_HANDLE_THICKNESS,
        frame->toolbox_rect.height
    };
    handles.side_panel = (Rectangle){
        frame->side_panel_rect.x - half_thickness,
        frame->side_panel_rect.y,
        WORKSPACE_RESIZE_HANDLE_THICKNESS,
        frame->side_panel_rect.height
    };
    handles.wave_panel = (Rectangle){
        frame->wave_rect.x,
        frame->wave_rect.y - half_thickness,
        frame->wave_rect.width,
        WORKSPACE_RESIZE_HANDLE_THICKNESS
    };
    return handles;
}

WorkspaceResizeHandle workspace_layout_hit_test_handle(const WorkspaceResizeHandles *handles, Vector2 mouse_pos) {
    if (!handles) {
        return WORKSPACE_RESIZE_HANDLE_NONE;
    }

    if (CheckCollisionPointRec(mouse_pos, handles->wave_panel)) {
        return WORKSPACE_RESIZE_HANDLE_WAVE_PANEL;
    }
    if (CheckCollisionPointRec(mouse_pos, handles->side_panel)) {
        return WORKSPACE_RESIZE_HANDLE_SIDE_PANEL;
    }
    if (CheckCollisionPointRec(mouse_pos, handles->toolbox)) {
        return WORKSPACE_RESIZE_HANDLE_TOOLBOX;
    }

    return WORKSPACE_RESIZE_HANDLE_NONE;
}

bool workspace_layout_apply_drag(WorkspaceLayoutPrefs *prefs, WorkspaceResizeHandle handle, Vector2 mouse_pos, int window_width, int window_height) {
    WorkspaceLayoutPrefs previous;
    float body_bottom;

    if (!prefs || handle == WORKSPACE_RESIZE_HANDLE_NONE) {
        return false;
    }

    previous = *prefs;
    if (handle == WORKSPACE_RESIZE_HANDLE_TOOLBOX) {
        prefs->toolbox_width = mouse_pos.x;
    } else if (handle == WORKSPACE_RESIZE_HANDLE_SIDE_PANEL) {
        prefs->side_panel_width = (float)window_width - mouse_pos.x;
    } else if (handle == WORKSPACE_RESIZE_HANDLE_WAVE_PANEL) {
        body_bottom = (float)window_height - WORKSPACE_FOOTER_HEIGHT;
        prefs->wave_height = body_bottom - mouse_pos.y;
    }

    workspace_layout_sanitize_prefs(prefs, window_width, window_height);
    return float_changed(prefs->toolbox_width, previous.toolbox_width) ||
        float_changed(prefs->side_panel_width, previous.side_panel_width) ||
        float_changed(prefs->wave_height, previous.wave_height);
}

bool workspace_layout_load_prefs(WorkspaceLayoutPrefs *prefs) {
    char path[LAYOUT_PATH_MAX];
    FILE *file;
    char line[128];
    WorkspaceLayoutPrefs loaded;
    bool saw_toolbox;
    bool saw_side_panel;
    bool saw_wave;

    if (!prefs || !workspace_layout_path(path, sizeof(path))) {
        return false;
    }

    file = fopen(path, "rb");
    if (!file) {
        return false;
    }

    loaded = *prefs;
    saw_toolbox = false;
    saw_side_panel = false;
    saw_wave = false;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "toolbox_width=%f", &loaded.toolbox_width) == 1) {
            saw_toolbox = true;
            continue;
        }
        if (sscanf(line, "side_panel_width=%f", &loaded.side_panel_width) == 1) {
            saw_side_panel = true;
            continue;
        }
        if (sscanf(line, "wave_height=%f", &loaded.wave_height) == 1) {
            saw_wave = true;
        }
    }

    fclose(file);
    if (!saw_toolbox || !saw_side_panel || !saw_wave) {
        return false;
    }

    *prefs = loaded;
    return true;
}

bool workspace_layout_save_prefs(const WorkspaceLayoutPrefs *prefs) {
    char path[LAYOUT_PATH_MAX];
    FILE *file;

    if (!prefs || !workspace_layout_path(path, sizeof(path))) {
        return false;
    }
    if (!ensure_layout_parent_dir(path)) {
        return false;
    }

    file = fopen(path, "wb");
    if (!file) {
        return false;
    }

    fprintf(file, "toolbox_width=%.2f\n", (double)prefs->toolbox_width);
    fprintf(file, "side_panel_width=%.2f\n", (double)prefs->side_panel_width);
    fprintf(file, "wave_height=%.2f\n", (double)prefs->wave_height);
    fclose(file);
    return true;
}
