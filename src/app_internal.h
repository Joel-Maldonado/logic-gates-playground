#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "app.h"

bool app_pin_is_input(const LogicPin *pin);
bool app_pin_is_output(const LogicPin *pin);
bool app_sink_has_connection(const AppContext *app, const LogicPin *pin);
LogicNode *app_primary_output_node(AppContext *app);
void app_record_waveforms(AppContext *app);
void app_compare_if_needed(AppContext *app);

#endif // APP_INTERNAL_H
