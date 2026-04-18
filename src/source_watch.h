#ifndef SOURCE_WATCH_H
#define SOURCE_WATCH_H

#include <stdbool.h>
#include <sys/types.h>
#include <time.h>
#include "app.h"

typedef struct {
    char path[APP_SOURCE_PATH_MAX];
    time_t modified_time;
    off_t size;
    bool active;
    uint8_t _padding[7];
} SourceWatch;

void source_watch_init(SourceWatch *watch);
bool source_watch_start(SourceWatch *watch, const char *path);
bool source_watch_load_circuit(AppContext *app, const char *path, const char *status, Rectangle canvas_rect);
bool source_watch_reload_if_changed(SourceWatch *watch, AppContext *app, Rectangle canvas_rect);
const char *source_watch_parse_load_path(int argc, char **argv);

#endif // SOURCE_WATCH_H
