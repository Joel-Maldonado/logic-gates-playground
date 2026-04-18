#include "source_watch.h"
#include "app_canvas.h"
#include "circuit_file.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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

void source_watch_init(SourceWatch *watch) {
    memset(watch, 0, sizeof(*watch));
}

bool source_watch_start(SourceWatch *watch, const char *path) {
    bool changed;

    if (!watch || !path) {
        return false;
    }

    snprintf(watch->path, sizeof(watch->path), "%s", path);
    watch->active = true;
    changed = false;
    return source_watch_refresh(watch, &changed);
}

bool source_watch_load_circuit(AppContext *app, const char *path, const char *status, Rectangle canvas_rect) {
    char error_message[APP_STATUS_MESSAGE_MAX];
    char status_message[APP_STATUS_MESSAGE_MAX];

    if (!circuit_file_load(app, path, error_message, sizeof(error_message))) {
        snprintf(status_message, sizeof(status_message), "Load failed: %s", error_message);
        app_set_source_status(app, status_message);
        return false;
    }

    app_set_source_path(app, path);
    app_frame_graph_in_canvas(app, canvas_rect);
    app_set_source_status(app, status);
    return true;
}

bool source_watch_reload_if_changed(SourceWatch *watch, AppContext *app, Rectangle canvas_rect) {
    bool changed;

    changed = false;
    if (!source_watch_refresh(watch, &changed) || !changed) {
        return false;
    }

    return source_watch_load_circuit(app, watch->path, "Reloaded from file", canvas_rect);
}

const char *source_watch_parse_load_path(int argc, char **argv) {
    int index;

    for (index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--load") == 0 && index + 1 < argc) {
            return argv[index + 1];
        }
        if (argv[index][0] != '-') {
            return argv[index];
        }
    }

    return NULL;
}
