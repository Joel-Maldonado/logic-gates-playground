#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/app.h"
#include "../src/circuit_file.h"
#include "../src/logic.h"
#include "../src/ui.h"
#include "../src/workspace_layout.h"

static void test_gate_and(void) {
    LogicValue inputs_low[] = { LOGIC_LOW, LOGIC_LOW };
    LogicValue inputs_mixed[] = { LOGIC_LOW, LOGIC_HIGH };
    LogicValue inputs_high[] = { LOGIC_HIGH, LOGIC_HIGH };

    assert(logic_eval_gate(NODE_GATE_AND, inputs_low, 2) == LOGIC_LOW);
    assert(logic_eval_gate(NODE_GATE_AND, inputs_mixed, 2) == LOGIC_LOW);
    assert(logic_eval_gate(NODE_GATE_AND, inputs_high, 2) == LOGIC_HIGH);
    printf("test_gate_and passed!\n");
}

static void test_gate_or(void) {
    LogicValue inputs_low[] = { LOGIC_LOW, LOGIC_LOW };
    LogicValue inputs_mixed[] = { LOGIC_LOW, LOGIC_HIGH };
    LogicValue inputs_high[] = { LOGIC_HIGH, LOGIC_HIGH };

    assert(logic_eval_gate(NODE_GATE_OR, inputs_low, 2) == LOGIC_LOW);
    assert(logic_eval_gate(NODE_GATE_OR, inputs_mixed, 2) == LOGIC_HIGH);
    assert(logic_eval_gate(NODE_GATE_OR, inputs_high, 2) == LOGIC_HIGH);
    printf("test_gate_or passed!\n");
}

static void test_simple_circuit(void) {
    LogicGraph graph;
    LogicNode *a;
    LogicNode *b;
    LogicNode *and_gate;
    LogicNode *output;

    logic_init_graph(&graph);
    a = logic_add_node(&graph, NODE_INPUT, "A");
    b = logic_add_node(&graph, NODE_INPUT, "B");
    and_gate = logic_add_node(&graph, NODE_GATE_AND, "AND1");
    output = logic_add_node(&graph, NODE_OUTPUT, "Z");

    logic_connect(&graph, &a->outputs[0], &and_gate->inputs[0]);
    logic_connect(&graph, &b->outputs[0], &and_gate->inputs[1]);
    logic_connect(&graph, &and_gate->outputs[0], &output->inputs[0]);

    a->outputs[0].value = LOGIC_LOW;
    b->outputs[0].value = LOGIC_LOW;
    logic_evaluate(&graph);
    assert(output->inputs[0].value == LOGIC_LOW);

    a->outputs[0].value = LOGIC_HIGH;
    b->outputs[0].value = LOGIC_LOW;
    logic_evaluate(&graph);
    assert(output->inputs[0].value == LOGIC_LOW);

    a->outputs[0].value = LOGIC_HIGH;
    b->outputs[0].value = LOGIC_HIGH;
    logic_evaluate(&graph);
    assert(output->inputs[0].value == LOGIC_HIGH);
    printf("test_simple_circuit passed!\n");
}

static void test_truth_table(void) {
    LogicGraph graph;
    LogicNode *a;
    LogicNode *b;
    LogicNode *and_gate;
    LogicNode *output;
    TruthTable *table;

    logic_init_graph(&graph);
    a = logic_add_node(&graph, NODE_INPUT, "A");
    b = logic_add_node(&graph, NODE_INPUT, "B");
    and_gate = logic_add_node(&graph, NODE_GATE_AND, "AND1");
    output = logic_add_node(&graph, NODE_OUTPUT, "Z");

    logic_connect(&graph, &a->outputs[0], &and_gate->inputs[0]);
    logic_connect(&graph, &b->outputs[0], &and_gate->inputs[1]);
    logic_connect(&graph, &and_gate->outputs[0], &output->inputs[0]);

    table = logic_generate_truth_table(&graph);
    assert(table != NULL);
    assert(table->row_count == 4U);
    assert(table->input_count == 2U);
    assert(table->output_count == 1U);
    assert(table->data[0] == LOGIC_LOW);
    assert(table->data[1] == LOGIC_LOW);
    assert(table->data[2] == LOGIC_LOW);
    assert(table->data[9] == LOGIC_HIGH);
    assert(table->data[10] == LOGIC_HIGH);
    assert(table->data[11] == LOGIC_HIGH);

    logic_free_truth_table(table);
    printf("test_truth_table passed!\n");
}

static void test_expression(void) {
    LogicGraph graph;
    LogicNode *a;
    LogicNode *b;
    LogicNode *and_gate;
    LogicNode *output;
    char *expression;

    logic_init_graph(&graph);
    a = logic_add_node(&graph, NODE_INPUT, "A");
    b = logic_add_node(&graph, NODE_INPUT, "B");
    and_gate = logic_add_node(&graph, NODE_GATE_AND, "AND1");
    output = logic_add_node(&graph, NODE_OUTPUT, "Z");

    logic_connect(&graph, &a->outputs[0], &and_gate->inputs[0]);
    logic_connect(&graph, &b->outputs[0], &and_gate->inputs[1]);
    logic_connect(&graph, &and_gate->outputs[0], &output->inputs[0]);

    expression = logic_generate_expression(&graph, output);
    assert(expression != NULL);
    assert(strcmp(expression, "(A AND B)") == 0);

    free(expression);
    printf("test_expression passed!\n");
}

static void test_reconnect_replaces_existing_input(void) {
    LogicGraph graph;
    LogicNode *a;
    LogicNode *b;
    LogicNode *output;

    logic_init_graph(&graph);
    a = logic_add_node(&graph, NODE_INPUT, "A");
    b = logic_add_node(&graph, NODE_INPUT, "B");
    output = logic_add_node(&graph, NODE_OUTPUT, "Z");

    assert(logic_connect(&graph, &a->outputs[0], &output->inputs[0]));
    assert(logic_connect(&graph, &b->outputs[0], &output->inputs[0]));

    a->outputs[0].value = LOGIC_LOW;
    b->outputs[0].value = LOGIC_HIGH;
    logic_evaluate(&graph);
    assert(output->inputs[0].value == LOGIC_HIGH);
    assert(graph.net_count == 1U);
    printf("test_reconnect_replaces_existing_input passed!\n");
}

static void test_remove_node_removes_attached_nets(void) {
    LogicGraph graph;
    LogicNode *a;
    LogicNode *b;
    LogicNode *and_gate;
    LogicNode *output;

    logic_init_graph(&graph);
    a = logic_add_node(&graph, NODE_INPUT, "A");
    b = logic_add_node(&graph, NODE_INPUT, "B");
    and_gate = logic_add_node(&graph, NODE_GATE_AND, "AND1");
    output = logic_add_node(&graph, NODE_OUTPUT, "Z");

    assert(logic_connect(&graph, &a->outputs[0], &and_gate->inputs[0]));
    assert(logic_connect(&graph, &b->outputs[0], &and_gate->inputs[1]));
    assert(logic_connect(&graph, &and_gate->outputs[0], &output->inputs[0]));
    assert(graph.net_count == 3U);

    assert(logic_remove_node(&graph, and_gate));
    assert(and_gate->type == (NodeType)-1);
    assert(graph.net_count == 0U);
    printf("test_remove_node_removes_attached_nets passed!\n");
}

static void write_text_file(const char *path, const char *text) {
    FILE *file;

    file = fopen(path, "wb");
    assert(file != NULL);
    assert(fputs(text, file) >= 0);
    fclose(file);
}

static LogicNode *find_node_by_name(AppContext *app, const char *name) {
    uint32_t i;

    for (i = 0; i < app->graph.node_count; i++) {
        LogicNode *node;

        node = &app->graph.nodes[i];
        if (node->type == (NodeType)-1 || !node->name) {
            continue;
        }
        if (strcmp(node->name, name) == 0) {
            return node;
        }
    }

    return NULL;
}

static LogicValue table_output_value(const AppContext *app, uint32_t row_index, uint32_t output_index) {
    uint32_t cols;

    assert(app->current_table != NULL);
    cols = (uint32_t)app->current_table->input_count + (uint32_t)app->current_table->output_count;
    return app->current_table->data[(row_index * cols) + (uint32_t)app->current_table->input_count + output_index];
}

static void test_app_default_names(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *z;
    LogicNode *z1;
    LogicNode *and_gate;
    LogicNode *clock;

    app_init(&app);
    a = app_add_node(&app, NODE_INPUT, (Vector2){ 100.0f, 100.0f });
    b = app_add_node(&app, NODE_INPUT, (Vector2){ 100.0f, 160.0f });
    z = app_add_node(&app, NODE_OUTPUT, (Vector2){ 220.0f, 100.0f });
    z1 = app_add_node(&app, NODE_OUTPUT, (Vector2){ 220.0f, 160.0f });
    and_gate = app_add_node(&app, NODE_GATE_AND, (Vector2){ 160.0f, 130.0f });
    clock = app_add_node(&app, NODE_GATE_CLOCK, (Vector2){ 160.0f, 200.0f });

    assert(strcmp(a->name, "A") == 0);
    assert(strcmp(b->name, "B") == 0);
    assert(strcmp(z->name, "Z") == 0);
    assert(strcmp(z1->name, "Z1") == 0);
    assert(strcmp(and_gate->name, "AND1") == 0);
    assert(strcmp(clock->name, "CLK1") == 0);

    app_clear_graph(&app);
    printf("test_app_default_names passed!\n");
}

static void test_interactive_construction_flow(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *out;
    LogicNode *and_gate;
    char *explanation;

    app_init(&app);
    a = app_add_node(&app, NODE_INPUT, (Vector2){ 140.0f, 160.0f });
    out = app_add_node(&app, NODE_OUTPUT, (Vector2){ 520.0f, 220.0f });
    assert(app_connect_pins(&app, &a->outputs[0], &out->inputs[0]));

    b = app_add_node(&app, NODE_INPUT, (Vector2){ 140.0f, 260.0f });
    and_gate = app_add_node(&app, NODE_GATE_AND, (Vector2){ 320.0f, 220.0f });
    assert(app_connect_pins(&app, &a->outputs[0], &and_gate->inputs[0]));
    assert(app_connect_pins(&app, &b->outputs[0], &and_gate->inputs[1]));
    assert(app_connect_pins(&app, &and_gate->outputs[0], &out->inputs[0]));

    assert(strcmp(app.current_expression, "(A AND B)") == 0);
    assert(table_output_value(&app, 0U, 0U) == LOGIC_LOW);
    assert(table_output_value(&app, 1U, 0U) == LOGIC_LOW);
    assert(table_output_value(&app, 2U, 0U) == LOGIC_LOW);
    assert(table_output_value(&app, 3U, 0U) == LOGIC_HIGH);

    app_select_row(&app, 2U);
    app_apply_selected_row_to_inputs(&app);
    assert(and_gate->outputs[0].value == LOGIC_LOW);
    assert(out->inputs[0].value == LOGIC_LOW);
    assert(app.view_ctx.row_valid);
    assert(app.view_ctx.live_row_index == 2U);
    explanation = app_get_node_explanation(&app, and_gate);
    assert(explanation != NULL);
    assert(strstr(explanation, "at least one input is 0") != NULL);
    free(explanation);

    app_select_row(&app, 3U);
    app_apply_selected_row_to_inputs(&app);
    assert(and_gate->outputs[0].value == LOGIC_HIGH);
    assert(out->inputs[0].value == LOGIC_HIGH);
    assert(app.view_ctx.row_valid);
    assert(app.view_ctx.live_row_index == 3U);

    assert(app_toggle_input_value(&app, a));
    assert(a->outputs[0].value == LOGIC_LOW);
    assert(and_gate->outputs[0].value == LOGIC_LOW);
    assert(app.view_ctx.row_valid);
    assert(app.view_ctx.live_row_index == 1U);

    app_clear_graph(&app);
    printf("test_interactive_construction_flow passed!\n");
}

static void test_compare_mode_without_target(void) {
    AppContext app;

    app_init(&app);
    assert(app_add_node(&app, NODE_INPUT, (Vector2){ 100.0f, 100.0f }) != NULL);
    assert(app_add_node(&app, NODE_OUTPUT, (Vector2){ 220.0f, 100.0f }) != NULL);
    assert(app_connect_pins(&app, &app.graph.nodes[0].outputs[0], &app.graph.nodes[1].inputs[0]));

    app_set_mode(&app, MODE_COMPARE);
    assert(app.compare_status == APP_COMPARE_NO_TARGET);
    assert(!app.comparison_equivalent);

    app_clear_graph(&app);
    printf("test_compare_mode_without_target passed!\n");
}

static void test_circuit_file_load(void) {
    AppContext app;
    char temp_path[] = "/tmp/mlvd-test-XXXXXX";
    int fd;
    bool loaded;
    char error_message[128];
    LogicNode *output;

    fd = mkstemp(temp_path);
    assert(fd >= 0);
    close(fd);

    write_text_file(
        temp_path,
        "input A at 140,160\n"
        "input B at 140,260\n"
        "and G1 at 300,220\n"
        "output Z at 500,240\n"
        "wire A -> G1.in0\n"
        "wire B -> G1.in1\n"
        "wire G1.out0 -> Z.in0\n"
    );

    app_init(&app);
    loaded = circuit_file_load(&app, temp_path, error_message, sizeof(error_message));
    assert(loaded);
    assert(app.graph.node_count == 4U);
    assert(app.current_table != NULL);
    assert(app.current_table->row_count == 4U);

    output = find_node_by_name(&app, "Z");
    assert(output != NULL);
    assert(strcmp(output->name, "Z") == 0);
    assert(output->pos.x == 500.0f);
    assert(output->pos.y == 240.0f);
    assert(strcmp(app.current_expression, "(A AND B)") == 0);

    unlink(temp_path);
    app_clear_graph(&app);
    printf("test_circuit_file_load passed!\n");
}

static void test_circuit_file_load_failure_keeps_existing_graph(void) {
    AppContext app;
    char temp_path[] = "/tmp/mlvd-test-invalid-XXXXXX";
    int fd;
    bool loaded;
    char error_message[128];

    fd = mkstemp(temp_path);
    assert(fd >= 0);
    close(fd);

    write_text_file(
        temp_path,
        "input A\n"
        "output Z\n"
        "wire Missing -> Z.in0\n"
    );

    app_init(&app);
    assert(app_add_named_node(&app, NODE_INPUT, "ExistingIn", (Vector2){ 100.0f, 100.0f }) != NULL);
    assert(app_add_named_node(&app, NODE_OUTPUT, "ExistingOut", (Vector2){ 200.0f, 100.0f }) != NULL);
    assert(logic_connect(&app.graph, &app.graph.nodes[0].outputs[0], &app.graph.nodes[1].inputs[0]));
    app_update_logic(&app);

    loaded = circuit_file_load(&app, temp_path, error_message, sizeof(error_message));
    assert(!loaded);
    assert(strstr(error_message, "unknown node") != NULL);
    assert(app.graph.node_count == 2U);
    assert(strcmp(app.graph.nodes[0].name, "ExistingIn") == 0);

    unlink(temp_path);
    app_clear_graph(&app);
    printf("test_circuit_file_load_failure_keeps_existing_graph passed!\n");
}

static void test_view_context_matches_live_state(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *out;
    LogicNode *and_gate;

    app_init(&app);
    a = app_add_node(&app, NODE_INPUT, (Vector2){ 140.0f, 160.0f });
    b = app_add_node(&app, NODE_INPUT, (Vector2){ 140.0f, 260.0f });
    and_gate = app_add_node(&app, NODE_GATE_AND, (Vector2){ 320.0f, 220.0f });
    out = app_add_node(&app, NODE_OUTPUT, (Vector2){ 520.0f, 220.0f });
    assert(app_connect_pins(&app, &a->outputs[0], &and_gate->inputs[0]));
    assert(app_connect_pins(&app, &b->outputs[0], &and_gate->inputs[1]));
    assert(app_connect_pins(&app, &and_gate->outputs[0], &out->inputs[0]));

    a->outputs[0].value = LOGIC_HIGH;
    b->outputs[0].value = LOGIC_HIGH;
    logic_evaluate(&app.graph);
    app_compute_view_context(&app);

    assert(app.view_ctx.row_valid);
    assert(app.view_ctx.live_row_index == 3U);

    app.selected_row = 0U;
    app_compute_view_context(&app);
    assert(app.view_ctx.live_row_index == 3U);
    assert(a->outputs[0].value == LOGIC_HIGH);
    assert(b->outputs[0].value == LOGIC_HIGH);

    app.selected_node = and_gate;
    app_compute_view_context(&app);
    assert(app.view_ctx.output_valid);
    assert(app.view_ctx.live_output == LOGIC_HIGH);

    app_clear_graph(&app);
    printf("test_view_context_matches_live_state passed!\n");
}

static void test_equation_resolved(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *and_gate;
    LogicNode *out;
    char buf[256];

    app_init(&app);
    a = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    b = app_add_named_node(&app, NODE_INPUT, "B", (Vector2){ 140.0f, 260.0f });
    and_gate = app_add_named_node(&app, NODE_GATE_AND, "AND1", (Vector2){ 320.0f, 220.0f });
    out = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });
    assert(app_connect_pins(&app, &a->outputs[0], &and_gate->inputs[0]));
    assert(app_connect_pins(&app, &b->outputs[0], &and_gate->inputs[1]));
    assert(app_connect_pins(&app, &and_gate->outputs[0], &out->inputs[0]));

    a->outputs[0].value = LOGIC_HIGH;
    b->outputs[0].value = LOGIC_HIGH;
    logic_evaluate(&app.graph);

    assert(logic_format_equation_resolved(&app.graph, out, buf, sizeof(buf)));
    assert(strcmp(buf, "Z = (A AND B) = (1 AND 1) = 1") == 0);

    assert(logic_format_equation_resolved(&app.graph, and_gate, buf, sizeof(buf)));
    assert(strcmp(buf, "AND1 = (A AND B) = (1 AND 1) = 1") == 0);

    a->outputs[0].value = LOGIC_LOW;
    logic_evaluate(&app.graph);
    assert(logic_format_equation_resolved(&app.graph, out, buf, sizeof(buf)));
    assert(strcmp(buf, "Z = (A AND B) = (0 AND 1) = 0") == 0);

    app_clear_graph(&app);
    printf("test_equation_resolved passed!\n");
}

static void test_snap_node_position_keeps_single_pin_nodes_on_grid(void) {
    Vector2 snapped;

    snapped = app_snap_node_position((Vector2){ 143.0f, 213.0f }, NODE_OUTPUT);
    assert(snapped.x == 140.0f);
    assert(snapped.y == 200.0f);

    printf("test_snap_node_position_keeps_single_pin_nodes_on_grid passed!\n");
}

static void test_snap_node_position_centers_tall_gates(void) {
    Vector2 gate_snapped;

    gate_snapped = app_snap_node_position((Vector2){ 143.0f, 213.0f }, NODE_GATE_AND);

    assert(gate_snapped.x == 140.0f);
    assert(gate_snapped.y == 210.0f);
    assert(((int)gate_snapped.y % 20) == 10);
    assert(((int)(gate_snapped.y + 30.0f) % 20) == 0);

    printf("test_snap_node_position_centers_tall_gates passed!\n");
}

static void test_delete_selected_wire(void) {
    AppContext app;
    LogicNode *input;
    LogicNode *output;

    app_init(&app);
    input = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    output = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });

    assert(input != NULL);
    assert(output != NULL);
    assert(app_connect_pins(&app, &input->outputs[0], &output->inputs[0]));
    assert(app.graph.net_count == 1U);

    app_select_wire_by_sink(&app, &output->inputs[0]);
    assert(app.selected_wire_sink == &output->inputs[0]);
    assert(app.selected_node == NULL);
    assert(app_delete_selected_wire(&app));
    assert(app.selected_wire_sink == NULL);
    assert(app.graph.net_count == 0U);
    assert(!app_delete_selected_wire(&app));

    app_clear_graph(&app);
    printf("test_delete_selected_wire passed!\n");
}

static void test_ui_get_wire_at_uses_rendered_wire_path(void) {
    AppContext app;
    Rectangle canvas;
    LogicNode *input;
    LogicNode *output;
    LogicPin *hit_wire;

    app_init(&app);
    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    input = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    output = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });

    assert(input != NULL);
    assert(output != NULL);
    assert(app_connect_pins(&app, &input->outputs[0], &output->inputs[0]));

    hit_wire = ui_get_wire_at(
        &app,
        canvas,
        app_canvas_world_to_screen(&app, canvas, (Vector2){ 280.0f, 180.0f })
    );
    assert(hit_wire == &output->inputs[0]);

    app_clear_graph(&app);
    printf("test_ui_get_wire_at_uses_rendered_wire_path passed!\n");
}

static void assert_rect_inside(Rectangle inner, Rectangle outer) {
    assert(inner.x >= outer.x);
    assert(inner.y >= outer.y);
    assert(inner.width >= 0.0f);
    assert(inner.height >= 0.0f);
    assert(inner.x + inner.width <= outer.x + outer.width);
    assert(inner.y + inner.height <= outer.y + outer.height);
}

static void assert_float_close(float actual, float expected) {
    assert(fabsf(actual - expected) < 0.001f);
}

static void test_canvas_coordinate_transform_round_trip(void) {
    Rectangle canvas;
    Vector2 origin;
    Vector2 screen;
    Vector2 world;
    Vector2 round_trip;

    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    origin = (Vector2){ 96.0f, 84.0f };
    screen = (Vector2){ 404.0f, 278.0f };
    world = app_canvas_screen_to_world_at(origin, 1.75f, canvas, screen);
    round_trip = app_canvas_world_to_screen_at(origin, 1.75f, canvas, world);

    assert_float_close(round_trip.x, screen.x);
    assert_float_close(round_trip.y, screen.y);
    printf("test_canvas_coordinate_transform_round_trip passed!\n");
}

static void test_canvas_zoom_anchor_keeps_world_point_stable(void) {
    Rectangle canvas;
    Vector2 origin;
    Vector2 anchor;
    Vector2 anchor_world;
    Vector2 zoomed_origin;
    Vector2 anchor_world_after_zoom;

    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    origin = (Vector2){ 40.0f, 30.0f };
    anchor = (Vector2){ 392.0f, 266.0f };
    anchor_world = app_canvas_screen_to_world_at(origin, 1.0f, canvas, anchor);
    zoomed_origin = app_canvas_origin_after_zoom(origin, 1.0f, 2.1f, canvas, anchor);
    anchor_world_after_zoom = app_canvas_screen_to_world_at(zoomed_origin, 2.1f, canvas, anchor);

    assert_float_close(anchor_world_after_zoom.x, anchor_world.x);
    assert_float_close(anchor_world_after_zoom.y, anchor_world.y);
    printf("test_canvas_zoom_anchor_keeps_world_point_stable passed!\n");
}

static void test_canvas_zoom_clamps_to_supported_range(void) {
    assert_float_close(app_canvas_clamp_zoom(0.1f), APP_CANVAS_MIN_ZOOM);
    assert_float_close(app_canvas_clamp_zoom(1.4f), 1.4f);
    assert_float_close(app_canvas_clamp_zoom(4.2f), APP_CANVAS_MAX_ZOOM);
    printf("test_canvas_zoom_clamps_to_supported_range passed!\n");
}

static void test_canvas_pan_updates_origin_predictably(void) {
    AppContext app;

    app_init(&app);
    app.canvas_origin = (Vector2){ 120.0f, 80.0f };
    app.canvas_zoom = 2.0f;
    app_pan_canvas(&app, (Vector2){ 40.0f, -20.0f });

    assert_float_close(app.canvas_origin.x, 100.0f);
    assert_float_close(app.canvas_origin.y, 90.0f);
    printf("test_canvas_pan_updates_origin_predictably passed!\n");
}

static void test_ui_get_wire_at_tracks_canvas_viewport(void) {
    AppContext app;
    Rectangle canvas;
    LogicNode *input;
    LogicNode *output;
    LogicPin *hit_wire;
    Vector2 hit_screen_pos;

    app_init(&app);
    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    input = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    output = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });

    assert(input != NULL);
    assert(output != NULL);
    assert(app_connect_pins(&app, &input->outputs[0], &output->inputs[0]));

    app.canvas_origin = (Vector2){ 60.0f, 40.0f };
    app.canvas_zoom = 1.6f;
    hit_screen_pos = app_canvas_world_to_screen(&app, canvas, (Vector2){ 280.0f, 180.0f });
    hit_wire = ui_get_wire_at(&app, canvas, hit_screen_pos);

    assert(hit_wire == &output->inputs[0]);
    app_clear_graph(&app);
    printf("test_ui_get_wire_at_tracks_canvas_viewport passed!\n");
}

static void test_ui_get_pin_at_tracks_canvas_zoom(void) {
    AppContext app;
    Rectangle canvas;
    LogicNode *input;
    LogicPin *hit_pin;
    Vector2 hit_screen_pos;

    app_init(&app);
    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    input = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });

    assert(input != NULL);

    app.canvas_origin = (Vector2){ 20.0f, 10.0f };
    app.canvas_zoom = 2.0f;
    hit_screen_pos = app_canvas_world_to_screen(&app, canvas, (Vector2){ 200.0f, 180.0f });
    hit_pin = ui_get_pin_at(&app, canvas, hit_screen_pos);

    assert(hit_pin == &input->outputs[0]);
    app_clear_graph(&app);
    printf("test_ui_get_pin_at_tracks_canvas_zoom passed!\n");
}

static void test_canvas_snap_uses_world_coordinates_after_navigation(void) {
    AppContext app;
    Rectangle canvas;
    Vector2 screen_pos;
    Vector2 world_pos;
    Vector2 snapped;

    app_init(&app);
    canvas = (Rectangle){ 132.0f, 50.0f, 720.0f, 480.0f };
    app.canvas_origin = (Vector2){ 80.0f, 40.0f };
    app.canvas_zoom = 2.0f;
    screen_pos = (Vector2){ 402.0f, 274.0f };
    world_pos = app_canvas_screen_to_world(&app, canvas, screen_pos);
    snapped = app_snap_node_position(world_pos, NODE_GATE_AND);

    assert_float_close(snapped.x, 200.0f);
    assert_float_close(snapped.y, 150.0f);
    printf("test_canvas_snap_uses_world_coordinates_after_navigation passed!\n");
}

static void test_ui_measure_context_panel_stays_within_min_window_bounds(void) {
    AppContext app;
    Rectangle side_panel;
    UiContextPanelLayout layout;

    app_init(&app);
    side_panel = (Rectangle){ 770.0f, 50.0f, 330.0f, 646.0f };
    layout = ui_measure_context_panel(&app, side_panel);

    assert_rect_inside(layout.status_rect, side_panel);
    assert_rect_inside(layout.equation_rect, side_panel);
    assert_rect_inside(layout.truth_table_rect, side_panel);
    assert_rect_inside(layout.why_rect, side_panel);
    assert(layout.compare_rect.width == 0.0f);
    assert(layout.kmap_rect.width == 0.0f);
    assert(!layout.show_compare);
    assert(!layout.show_kmap);

    app_clear_graph(&app);
    printf("test_ui_measure_context_panel_stays_within_min_window_bounds passed!\n");
}

static void test_ui_measure_context_panel_allocates_compare_and_kmap_sections(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *out;
    Rectangle side_panel;
    UiContextPanelLayout layout;

    app_init(&app);
    a = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    b = app_add_named_node(&app, NODE_INPUT, "B", (Vector2){ 140.0f, 260.0f });
    out = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });

    assert(a != NULL);
    assert(b != NULL);
    assert(out != NULL);
    assert(app_connect_pins(&app, &a->outputs[0], &out->inputs[0]));
    app_set_mode(&app, MODE_COMPARE);

    side_panel = (Rectangle){ 770.0f, 50.0f, 330.0f, 646.0f };
    layout = ui_measure_context_panel(&app, side_panel);

    assert(layout.show_compare);
    assert(layout.show_kmap);
    assert(layout.compare_rect.height > 0.0f);
    assert(layout.kmap_rect.height > 0.0f);
    assert_rect_inside(layout.compare_rect, side_panel);
    assert_rect_inside(layout.kmap_rect, side_panel);
    assert_rect_inside(layout.why_rect, side_panel);
    assert(layout.truth_table_rect.height > 0.0f);

    app_clear_graph(&app);
    printf("test_ui_measure_context_panel_allocates_compare_and_kmap_sections passed!\n");
}

static void test_ui_context_truth_table_row_rect_matches_first_visible_row(void) {
    AppContext app;
    LogicNode *a;
    LogicNode *b;
    LogicNode *out;
    Rectangle side_panel;
    UiContextPanelLayout layout;
    Rectangle row_rect;

    app_init(&app);
    a = app_add_named_node(&app, NODE_INPUT, "A", (Vector2){ 140.0f, 160.0f });
    b = app_add_named_node(&app, NODE_INPUT, "B", (Vector2){ 140.0f, 260.0f });
    out = app_add_named_node(&app, NODE_OUTPUT, "Z", (Vector2){ 520.0f, 220.0f });

    assert(a != NULL);
    assert(b != NULL);
    assert(out != NULL);
    assert(app_connect_pins(&app, &a->outputs[0], &out->inputs[0]));

    side_panel = (Rectangle){ 770.0f, 50.0f, 330.0f, 646.0f };
    layout = ui_measure_context_panel(&app, side_panel);

    assert(layout.visible_truth_rows > 0U);
    assert(ui_context_truth_table_row_rect(&app, &layout, 0U, &row_rect));
    assert_float_close(row_rect.x, layout.truth_table_rect.x + 10.0f);
    assert_float_close(row_rect.y, layout.truth_table_rect.y + 59.0f);
    assert_float_close(row_rect.width, layout.truth_table_rect.width - 20.0f);
    assert_float_close(row_rect.height, 20.0f);

    app_clear_graph(&app);
    printf("test_ui_context_truth_table_row_rect_matches_first_visible_row passed!\n");
}

static void test_workspace_layout_clamps_panel_sizes(void) {
    WorkspaceLayoutPrefs prefs;
    WorkspaceFrame frame;

    workspace_layout_init_defaults(&prefs);
    prefs.toolbox_width = 400.0f;
    prefs.side_panel_width = 900.0f;
    prefs.wave_height = 900.0f;

    workspace_layout_sanitize_prefs(&prefs, WORKSPACE_WINDOW_MIN_WIDTH, WORKSPACE_WINDOW_MIN_HEIGHT);
    frame = workspace_layout_compute_frame(&prefs, WORKSPACE_WINDOW_MIN_WIDTH, WORKSPACE_WINDOW_MIN_HEIGHT);

    assert(prefs.toolbox_width >= WORKSPACE_TOOLBOX_MIN_WIDTH);
    assert(prefs.toolbox_width <= WORKSPACE_TOOLBOX_MAX_WIDTH);
    assert(prefs.side_panel_width >= WORKSPACE_SIDE_PANEL_MIN_WIDTH);
    assert(prefs.side_panel_width <= WORKSPACE_SIDE_PANEL_MAX_WIDTH);
    assert(prefs.wave_height >= WORKSPACE_WAVE_MIN_HEIGHT);
    assert(prefs.wave_height <= WORKSPACE_WAVE_MAX_HEIGHT);
    assert(frame.canvas_rect.width >= WORKSPACE_CENTER_MIN_WIDTH);
    assert(frame.canvas_rect.height >= WORKSPACE_CANVAS_MIN_HEIGHT);

    printf("test_workspace_layout_clamps_panel_sizes passed!\n");
}

static void test_workspace_layout_drag_updates_each_panel(void) {
    WorkspaceLayoutPrefs prefs;
    WorkspaceFrame frame;

    workspace_layout_init_defaults(&prefs);

    assert(workspace_layout_apply_drag(
        &prefs,
        WORKSPACE_RESIZE_HANDLE_TOOLBOX,
        (Vector2){ 150.0f, 0.0f },
        1440,
        900
    ));
    assert_float_close(prefs.toolbox_width, 150.0f);

    assert(workspace_layout_apply_drag(
        &prefs,
        WORKSPACE_RESIZE_HANDLE_SIDE_PANEL,
        (Vector2){ 1000.0f, 0.0f },
        1440,
        900
    ));
    assert_float_close(prefs.side_panel_width, 440.0f);

    assert(workspace_layout_apply_drag(
        &prefs,
        WORKSPACE_RESIZE_HANDLE_WAVE_PANEL,
        (Vector2){ 0.0f, 540.0f },
        1440,
        900
    ));

    frame = workspace_layout_compute_frame(&prefs, 1440, 900);
    assert_float_close(frame.toolbox_rect.width, prefs.toolbox_width);
    assert_float_close(frame.side_panel_rect.width, prefs.side_panel_width);
    assert_float_close(frame.wave_rect.height, prefs.wave_height);

    printf("test_workspace_layout_drag_updates_each_panel passed!\n");
}

static void test_workspace_layout_save_and_load_round_trip(void) {
    WorkspaceLayoutPrefs saved;
    WorkspaceLayoutPrefs loaded;
    char temp_path[] = "/tmp/mlvd-layout-XXXXXX";
    int fd;

    fd = mkstemp(temp_path);
    assert(fd >= 0);
    close(fd);
    unlink(temp_path);

    assert(setenv("MLVD_LAYOUT_PATH", temp_path, 1) == 0);

    workspace_layout_init_defaults(&saved);
    saved.toolbox_width = 142.0f;
    saved.side_panel_width = 418.0f;
    saved.wave_height = 228.0f;

    assert(workspace_layout_save_prefs(&saved));

    workspace_layout_init_defaults(&loaded);
    loaded.toolbox_width = 0.0f;
    loaded.side_panel_width = 0.0f;
    loaded.wave_height = 0.0f;
    assert(workspace_layout_load_prefs(&loaded));
    assert_float_close(loaded.toolbox_width, saved.toolbox_width);
    assert_float_close(loaded.side_panel_width, saved.side_panel_width);
    assert_float_close(loaded.wave_height, saved.wave_height);

    unsetenv("MLVD_LAYOUT_PATH");
    unlink(temp_path);
    printf("test_workspace_layout_save_and_load_round_trip passed!\n");
}

static void test_toolbox_items_scale_with_panel_width(void) {
    Rectangle narrow_panel;
    Rectangle wide_panel;
    Rectangle narrow_item;
    Rectangle wide_item;
    Vector2 hit_point;

    narrow_panel = (Rectangle){ 0.0f, 50.0f, WORKSPACE_TOOLBOX_MIN_WIDTH, 646.0f };
    wide_panel = (Rectangle){ 0.0f, 50.0f, WORKSPACE_TOOLBOX_MAX_WIDTH, 646.0f };

    assert(ui_toolbox_item_rect(narrow_panel, 0, &narrow_item));
    assert(ui_toolbox_item_rect(wide_panel, 0, &wide_item));
    assert(wide_item.height > narrow_item.height);
    assert(wide_item.width > narrow_item.width);
    assert(wide_item.height <= 58.0f);

    hit_point = (Vector2){
        wide_item.x + (wide_item.width * 0.5f),
        wide_item.y + (wide_item.height * 0.5f)
    };
    assert(ui_toolbox_slot_at(wide_panel, hit_point) == 0);

    printf("test_toolbox_items_scale_with_panel_width passed!\n");
}

int main(void) {
    test_gate_and();
    test_gate_or();
    test_simple_circuit();
    test_truth_table();
    test_expression();
    test_reconnect_replaces_existing_input();
    test_remove_node_removes_attached_nets();
    test_app_default_names();
    test_interactive_construction_flow();
    test_compare_mode_without_target();
    test_circuit_file_load();
    test_circuit_file_load_failure_keeps_existing_graph();
    test_view_context_matches_live_state();
    test_equation_resolved();
    test_snap_node_position_keeps_single_pin_nodes_on_grid();
    test_snap_node_position_centers_tall_gates();
    test_delete_selected_wire();
    test_ui_get_wire_at_uses_rendered_wire_path();
    test_canvas_coordinate_transform_round_trip();
    test_canvas_zoom_anchor_keeps_world_point_stable();
    test_canvas_zoom_clamps_to_supported_range();
    test_canvas_pan_updates_origin_predictably();
    test_ui_get_wire_at_tracks_canvas_viewport();
    test_ui_get_pin_at_tracks_canvas_zoom();
    test_canvas_snap_uses_world_coordinates_after_navigation();
    test_ui_measure_context_panel_stays_within_min_window_bounds();
    test_ui_measure_context_panel_allocates_compare_and_kmap_sections();
    test_ui_context_truth_table_row_rect_matches_first_visible_row();
    test_workspace_layout_clamps_panel_sizes();
    test_workspace_layout_drag_updates_each_panel();
    test_workspace_layout_save_and_load_round_trip();
    test_toolbox_items_scale_with_panel_width();
    printf("All tests passed!\n");
    return 0;
}
