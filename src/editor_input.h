#ifndef EDITOR_INPUT_H
#define EDITOR_INPUT_H

#include "source_watch.h"
#include "topbar.h"
#include "workspace_layout.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

typedef struct {
    Vector2 click_start_pos;
    Vector2 pan_last_mouse;
    WorkspaceResizeHandle active_resize_handle;
    bool canvas_pan_active;
    bool canvas_pan_moved;
    bool click_moved;
    bool layout_save_pending;
    bool shortcuts_open;
} EditorInputState;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

void editor_input_init(EditorInputState *state);
void editor_input_process_frame(
    AppContext *app,
    EditorInputState *state,
    WorkspaceLayoutPrefs *layout_prefs,
    WorkspaceFrame *frame,
    WorkspaceResizeHandles *resize_handles,
    const TopbarLayout *topbar_layout,
    SourceWatch *source_watch
);
WorkspaceResizeHandle editor_input_active_resize_handle(const EditorInputState *state);
bool editor_input_shortcuts_open(const EditorInputState *state);

#endif // EDITOR_INPUT_H
