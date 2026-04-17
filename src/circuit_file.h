#ifndef CIRCUIT_FILE_H
#define CIRCUIT_FILE_H

#include <stdbool.h>
#include <stddef.h>
#include "app.h"

bool circuit_file_load(AppContext *app, const char *path, char *error_message, size_t error_message_size);

#endif // CIRCUIT_FILE_H
