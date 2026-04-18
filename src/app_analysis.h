#ifndef APP_ANALYSIS_H
#define APP_ANALYSIS_H

#include "app.h"

void app_update_kmap_grouping(AppContext *app);
void app_compare_with_target(AppContext *app, LogicGraph *target);
void app_compute_view_context(AppContext *app);
void app_apply_selected_row_to_inputs(AppContext *app);
bool app_toggle_input_value(AppContext *app, LogicNode *node);
char* app_get_node_explanation(AppContext *app, LogicNode *node);

#endif // APP_ANALYSIS_H
