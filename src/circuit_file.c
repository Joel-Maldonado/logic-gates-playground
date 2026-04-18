#include "circuit_file.h"
#include "circuit_layout.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSED_NAME_MAX 64

typedef struct {
    NodeType type;
    char name[PARSED_NAME_MAX];
    Vector2 position;
    bool has_position;
    uint8_t _padding[3];
} ParsedNode;

typedef struct {
    char node_name[PARSED_NAME_MAX];
    bool explicit_pin;
    bool is_output_pin;
    uint8_t pin_index;
} ParsedEndpoint;

typedef struct {
    ParsedEndpoint source;
    ParsedEndpoint sink;
} ParsedWire;

typedef struct {
    ParsedNode nodes[MAX_NODES];
    ParsedWire wires[MAX_NETS];
    uint32_t node_count;
    uint32_t wire_count;
} ParsedCircuit;

static void set_error(char *error_message, size_t error_message_size, const char *message, unsigned int line_number) {
    if (!error_message || error_message_size == 0U) {
        return;
    }

    if (line_number == 0U) {
        snprintf(error_message, error_message_size, "%s", message);
        return;
    }

    snprintf(error_message, error_message_size, "Line %u: %s", line_number, message);
}

static char *trim_whitespace(char *text) {
    size_t length;

    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    length = strlen(text);
    while (length > 0U && isspace((unsigned char)text[length - 1U])) {
        text[length - 1U] = '\0';
        length--;
    }

    return text;
}

static void strip_comment(char *text) {
    char *comment;

    comment = strchr(text, '#');
    if (comment) {
        *comment = '\0';
    }
}

static bool parse_node_type(const char *token, NodeType *type) {
    if (strcmp(token, "input") == 0) {
        *type = NODE_INPUT;
        return true;
    }
    if (strcmp(token, "output") == 0) {
        *type = NODE_OUTPUT;
        return true;
    }
    if (strcmp(token, "and") == 0) {
        *type = NODE_GATE_AND;
        return true;
    }
    if (strcmp(token, "or") == 0) {
        *type = NODE_GATE_OR;
        return true;
    }
    if (strcmp(token, "not") == 0) {
        *type = NODE_GATE_NOT;
        return true;
    }
    if (strcmp(token, "xor") == 0) {
        *type = NODE_GATE_XOR;
        return true;
    }
    if (strcmp(token, "dff") == 0) {
        *type = NODE_GATE_DFF;
        return true;
    }
    if (strcmp(token, "clock") == 0) {
        *type = NODE_GATE_CLOCK;
        return true;
    }

    return false;
}

static ParsedNode *find_parsed_node(ParsedCircuit *circuit, const char *name) {
    uint32_t i;

    for (i = 0; i < circuit->node_count; i++) {
        if (strcmp(circuit->nodes[i].name, name) == 0) {
            return &circuit->nodes[i];
        }
    }

    return NULL;
}

static int32_t find_parsed_node_index(const ParsedCircuit *circuit, const char *name) {
    uint32_t i;

    for (i = 0; i < circuit->node_count; i++) {
        if (strcmp(circuit->nodes[i].name, name) == 0) {
            return (int32_t)i;
        }
    }

    return -1;
}

static LogicNode *rebase_node_pointer(const AppContext *source_app, AppContext *dest_app, LogicNode *node) {
    ptrdiff_t node_index;

    if (!node) {
        return NULL;
    }

    node_index = node - source_app->graph.nodes;
    if (node_index < 0 || (uint32_t)node_index >= source_app->graph.node_count) {
        return NULL;
    }

    return &dest_app->graph.nodes[node_index];
}

static LogicPin *rebase_pin_pointer(const AppContext *source_app, AppContext *dest_app, LogicPin *pin) {
    uint32_t node_index;

    if (!pin) {
        return NULL;
    }

    for (node_index = 0; node_index < source_app->graph.node_count; node_index++) {
        const LogicNode *source_node;
        LogicNode *dest_node;

        source_node = &source_app->graph.nodes[node_index];
        dest_node = &dest_app->graph.nodes[node_index];

        if (pin >= source_node->inputs && pin < source_node->inputs + MAX_PINS) {
            ptrdiff_t pin_index;

            pin_index = pin - source_node->inputs;
            return &dest_node->inputs[pin_index];
        }
        if (pin >= source_node->outputs && pin < source_node->outputs + MAX_PINS) {
            ptrdiff_t pin_index;

            pin_index = pin - source_node->outputs;
            return &dest_node->outputs[pin_index];
        }
    }

    return NULL;
}

static void rebase_app_pointers(const AppContext *source_app, AppContext *dest_app) {
    uint32_t i;

    for (i = 0; i < dest_app->graph.node_count; i++) {
        LogicNode *node;
        uint8_t pin_index;

        node = &dest_app->graph.nodes[i];
        for (pin_index = 0; pin_index < MAX_PINS; pin_index++) {
            node->inputs[pin_index].node = node;
            node->outputs[pin_index].node = node;
        }
    }

    for (i = 0; i < dest_app->graph.net_count; i++) {
        LogicNet *net;
        uint8_t sink_index;

        net = &dest_app->graph.nets[i];
        net->source = rebase_pin_pointer(source_app, dest_app, net->source);
        for (sink_index = 0; sink_index < net->sink_count; sink_index++) {
            net->sinks[sink_index] = rebase_pin_pointer(source_app, dest_app, net->sinks[sink_index]);
        }
    }

    if (dest_app->current_table) {
        for (i = 0; i < dest_app->current_table->input_count; i++) {
            dest_app->current_table->inputs[i] =
                rebase_node_pointer(source_app, dest_app, dest_app->current_table->inputs[i]);
        }
        for (i = 0; i < dest_app->current_table->output_count; i++) {
            dest_app->current_table->outputs[i] =
                rebase_node_pointer(source_app, dest_app, dest_app->current_table->outputs[i]);
        }
    }

    dest_app->drag_node = rebase_node_pointer(source_app, dest_app, dest_app->drag_node);
    dest_app->selected_node = rebase_node_pointer(source_app, dest_app, dest_app->selected_node);
    dest_app->active_pin = rebase_pin_pointer(source_app, dest_app, dest_app->active_pin);
    dest_app->wire_drag_pin = rebase_pin_pointer(source_app, dest_app, dest_app->wire_drag_pin);
    dest_app->wire_hover_pin = rebase_pin_pointer(source_app, dest_app, dest_app->wire_hover_pin);
    dest_app->selected_wire_sink = rebase_pin_pointer(source_app, dest_app, dest_app->selected_wire_sink);
    dest_app->divergence_node = rebase_node_pointer(source_app, dest_app, dest_app->divergence_node);
    dest_app->view_ctx.selected_node = rebase_node_pointer(source_app, dest_app, dest_app->view_ctx.selected_node);
}

static bool parse_endpoint(const char *text, ParsedEndpoint *endpoint) {
    char buffer[PARSED_NAME_MAX];
    char *dot;
    char *endptr;
    long index;

    memset(endpoint, 0, sizeof(*endpoint));
    if (strlen(text) >= sizeof(buffer)) {
        return false;
    }

    snprintf(buffer, sizeof(buffer), "%s", text);
    dot = strchr(buffer, '.');
    if (!dot) {
        snprintf(endpoint->node_name, sizeof(endpoint->node_name), "%s", buffer);
        endpoint->explicit_pin = false;
        endpoint->pin_index = 0U;
        return true;
    }

    *dot = '\0';
    dot++;
    snprintf(endpoint->node_name, sizeof(endpoint->node_name), "%s", buffer);

    if (strncmp(dot, "out", 3) == 0) {
        endpoint->is_output_pin = true;
        dot += 3;
    } else if (strncmp(dot, "in", 2) == 0) {
        endpoint->is_output_pin = false;
        dot += 2;
    } else {
        return false;
    }

    if (*dot == '\0') {
        return false;
    }

    index = strtol(dot, &endptr, 10);
    if (*endptr != '\0' || index < 0L || index > 255L) {
        return false;
    }

    endpoint->explicit_pin = true;
    endpoint->pin_index = (uint8_t)index;
    return true;
}

static bool parse_position_clause(const char *text, Vector2 *position) {
    float x;
    float y;

    if (sscanf(text, "at %f , %f", &x, &y) == 2) {
        position->x = x;
        position->y = y;
        return true;
    }
    if (sscanf(text, "at %f,%f", &x, &y) == 2) {
        position->x = x;
        position->y = y;
        return true;
    }

    return false;
}

static bool parse_node_line(ParsedCircuit *circuit, char *line, unsigned int line_number, char *error_message, size_t error_message_size) {
    char *kind_token;
    char *name_token;
    char *rest;
    NodeType type;
    ParsedNode *node;

    kind_token = strtok(line, " \t");
    name_token = strtok(NULL, " \t");
    rest = strtok(NULL, "");

    if (!kind_token || !name_token) {
        set_error(error_message, error_message_size, "node declaration needs a type and name", line_number);
        return false;
    }
    if (!parse_node_type(kind_token, &type)) {
        set_error(error_message, error_message_size, "unknown node type", line_number);
        return false;
    }
    if (find_parsed_node(circuit, name_token)) {
        set_error(error_message, error_message_size, "duplicate node name", line_number);
        return false;
    }
    if (circuit->node_count >= MAX_NODES) {
        set_error(error_message, error_message_size, "too many nodes", line_number);
        return false;
    }

    node = &circuit->nodes[circuit->node_count++];
    memset(node, 0, sizeof(*node));
    node->type = type;
    snprintf(node->name, sizeof(node->name), "%s", name_token);

    if (rest) {
        rest = trim_whitespace(rest);
        if (*rest != '\0' && !parse_position_clause(rest, &node->position)) {
            set_error(error_message, error_message_size, "expected optional position like 'at 140,160'", line_number);
            return false;
        }
        node->has_position = (*rest != '\0');
    }

    return true;
}

static bool parse_wire_line(ParsedCircuit *circuit, char *line, unsigned int line_number, char *error_message, size_t error_message_size) {
    char *source_token;
    char *arrow_token;
    char *sink_token;
    ParsedWire *wire;

    source_token = strtok(line, " \t");
    arrow_token = strtok(NULL, " \t");
    sink_token = strtok(NULL, " \t");

    if (!source_token || !arrow_token || !sink_token || strcmp(arrow_token, "->") != 0) {
        set_error(error_message, error_message_size, "wire declaration must look like 'wire A -> G1.in0'", line_number);
        return false;
    }
    if (circuit->wire_count >= MAX_NETS) {
        set_error(error_message, error_message_size, "too many wires", line_number);
        return false;
    }

    wire = &circuit->wires[circuit->wire_count++];
    memset(wire, 0, sizeof(*wire));
    if (!parse_endpoint(source_token, &wire->source)) {
        set_error(error_message, error_message_size, "invalid source endpoint", line_number);
        return false;
    }
    if (!parse_endpoint(sink_token, &wire->sink)) {
        set_error(error_message, error_message_size, "invalid sink endpoint", line_number);
        return false;
    }

    return true;
}

static bool parse_circuit_text(const char *text, ParsedCircuit *circuit, char *error_message, size_t error_message_size) {
    char *buffer;
    char *cursor;
    unsigned int line_number;

    memset(circuit, 0, sizeof(*circuit));
    buffer = strdup(text);
    if (!buffer) {
        set_error(error_message, error_message_size, "out of memory", 0U);
        return false;
    }

    cursor = buffer;
    line_number = 0U;
    while (cursor && *cursor != '\0') {
        char *line_end;
        char *line;

        line_end = strchr(cursor, '\n');
        if (line_end) {
            *line_end = '\0';
        }

        line_number++;
        line = trim_whitespace(cursor);
        strip_comment(line);
        line = trim_whitespace(line);

        if (*line != '\0') {
            if (strncmp(line, "wire ", 5) == 0) {
                if (!parse_wire_line(circuit, line + 5, line_number, error_message, error_message_size)) {
                    free(buffer);
                    return false;
                }
            } else if (!parse_node_line(circuit, line, line_number, error_message, error_message_size)) {
                free(buffer);
                return false;
            }
        }

        cursor = line_end ? (line_end + 1) : NULL;
    }

    free(buffer);
    return true;
}

static void parsed_node_pin_counts(const ParsedNode *node, uint8_t *input_count, uint8_t *output_count) {
    if (!node || !input_count || !output_count) {
        return;
    }

    if (node->type == NODE_INPUT || node->type == NODE_GATE_CLOCK) {
        *input_count = 0U;
        *output_count = 1U;
        return;
    }
    if (node->type == NODE_OUTPUT) {
        *input_count = 1U;
        *output_count = 0U;
        return;
    }
    if (node->type == NODE_GATE_NOT) {
        *input_count = 1U;
        *output_count = 1U;
        return;
    }
    if (node->type == NODE_GATE_DFF) {
        *input_count = 2U;
        *output_count = 1U;
        return;
    }

    *input_count = 2U;
    *output_count = 1U;
}

static bool resolve_parsed_source_pin(const ParsedNode *node, const ParsedEndpoint *endpoint, uint8_t *pin_index) {
    uint8_t output_count;
    uint8_t input_count;

    if (!node || !pin_index) {
        return false;
    }

    parsed_node_pin_counts(node, &input_count, &output_count);
    if (endpoint->explicit_pin) {
        if (!endpoint->is_output_pin || endpoint->pin_index >= output_count) {
            return false;
        }
        *pin_index = endpoint->pin_index;
        return true;
    }
    if (output_count != 1U) {
        return false;
    }
    *pin_index = 0U;
    return true;
}

static bool resolve_parsed_sink_pin(const ParsedNode *node, const ParsedEndpoint *endpoint, uint8_t *pin_index) {
    uint8_t output_count;
    uint8_t input_count;

    if (!node || !pin_index) {
        return false;
    }

    parsed_node_pin_counts(node, &input_count, &output_count);
    if (endpoint->explicit_pin) {
        if (endpoint->is_output_pin || endpoint->pin_index >= input_count) {
            return false;
        }
        *pin_index = endpoint->pin_index;
        return true;
    }
    if (input_count != 1U) {
        return false;
    }
    *pin_index = 0U;
    return true;
}

static bool build_layout_spec(
    const ParsedCircuit *circuit,
    CircuitLayoutNode *layout_nodes,
    CircuitLayoutEdge *layout_edges,
    uint32_t *layout_edge_count,
    char *error_message,
    size_t error_message_size
) {
    uint32_t node_index;
    uint32_t wire_index;

    if (!circuit || !layout_nodes || !layout_edges || !layout_edge_count) {
        return false;
    }

    for (node_index = 0U; node_index < circuit->node_count; node_index++) {
        uint8_t input_count;
        uint8_t output_count;

        parsed_node_pin_counts(&circuit->nodes[node_index], &input_count, &output_count);
        layout_nodes[node_index].type = circuit->nodes[node_index].type;
        layout_nodes[node_index].name = circuit->nodes[node_index].name;
        layout_nodes[node_index].input_count = input_count;
        layout_nodes[node_index].output_count = output_count;
    }

    *layout_edge_count = 0U;
    for (wire_index = 0U; wire_index < circuit->wire_count; wire_index++) {
        const ParsedWire *parsed_wire;
        int32_t source_index;
        int32_t sink_index;
        uint8_t source_pin_index;
        uint8_t sink_pin_index;

        parsed_wire = &circuit->wires[wire_index];
        source_index = find_parsed_node_index(circuit, parsed_wire->source.node_name);
        sink_index = find_parsed_node_index(circuit, parsed_wire->sink.node_name);
        if (source_index < 0 || sink_index < 0) {
            set_error(error_message, error_message_size, "wire references an unknown node", 0U);
            return false;
        }
        if (!resolve_parsed_source_pin(&circuit->nodes[(uint32_t)source_index], &parsed_wire->source, &source_pin_index)) {
            set_error(error_message, error_message_size, "wire source pin is invalid", 0U);
            return false;
        }
        if (!resolve_parsed_sink_pin(&circuit->nodes[(uint32_t)sink_index], &parsed_wire->sink, &sink_pin_index)) {
            set_error(error_message, error_message_size, "wire sink pin is invalid", 0U);
            return false;
        }

        layout_edges[*layout_edge_count].source_node_index = (uint32_t)source_index;
        layout_edges[*layout_edge_count].source_pin_index = source_pin_index;
        layout_edges[*layout_edge_count].sink_node_index = (uint32_t)sink_index;
        layout_edges[*layout_edge_count].sink_pin_index = sink_pin_index;
        (*layout_edge_count)++;
    }

    return true;
}

static bool apply_parsed_circuit(AppContext *app, const ParsedCircuit *circuit, char *error_message, size_t error_message_size) {
    AppContext staged_app;
    CircuitLayoutNode layout_nodes[MAX_NODES];
    CircuitLayoutEdge layout_edges[MAX_NETS];
    Vector2 layout_positions[MAX_NODES];
    uint32_t layout_edge_count;
    uint32_t i;

    app_init(&staged_app);
    staged_app.mode = app->mode;
    staged_app.active_tool = app->active_tool;
    staged_app.focused_panel = app->focused_panel;
    staged_app.target_graph = app->target_graph;
    staged_app.sim_speed = app->sim_speed;
    app_set_source_path(&staged_app, app->source_path);
    staged_app.source_live_reload = app->source_live_reload;

    if (!build_layout_spec(
            circuit,
            layout_nodes,
            layout_edges,
            &layout_edge_count,
            error_message,
            error_message_size
        )) {
        app_clear_graph(&staged_app);
        return false;
    }
    if (!circuit_layout_resolve_positions(
            layout_nodes,
            circuit->node_count,
            layout_edges,
            layout_edge_count,
            layout_positions
        )) {
        app_clear_graph(&staged_app);
        set_error(error_message, error_message_size, "could not lay out circuit", 0U);
        return false;
    }

    for (i = 0; i < circuit->node_count; i++) {
        const ParsedNode *parsed_node;
        Vector2 position;

        parsed_node = &circuit->nodes[i];
        position = layout_positions[i];
        position = app_snap_node_position(position, parsed_node->type);
        if (!app_add_named_node(&staged_app, parsed_node->type, parsed_node->name, position)) {
            app_clear_graph(&staged_app);
            set_error(error_message, error_message_size, "could not add node to graph", 0U);
            return false;
        }
    }

    for (i = 0; i < circuit->wire_count; i++) {
        LogicNode *source_node;
        LogicNode *sink_node;
        LogicPin *source_pin;
        LogicPin *sink_pin;

        if (layout_edges[i].source_node_index >= staged_app.graph.node_count ||
            layout_edges[i].sink_node_index >= staged_app.graph.node_count) {
            app_clear_graph(&staged_app);
            set_error(error_message, error_message_size, "wire references an unknown node", 0U);
            return false;
        }
        source_node = &staged_app.graph.nodes[layout_edges[i].source_node_index];
        sink_node = &staged_app.graph.nodes[layout_edges[i].sink_node_index];
        if (layout_edges[i].source_pin_index >= source_node->output_count) {
            app_clear_graph(&staged_app);
            set_error(error_message, error_message_size, "wire source pin is invalid", 0U);
            return false;
        }
        if (layout_edges[i].sink_pin_index >= sink_node->input_count) {
            app_clear_graph(&staged_app);
            set_error(error_message, error_message_size, "wire sink pin is invalid", 0U);
            return false;
        }
        source_pin = &source_node->outputs[layout_edges[i].source_pin_index];
        sink_pin = &sink_node->inputs[layout_edges[i].sink_pin_index];
        if (!logic_connect(&staged_app.graph, source_pin, sink_pin)) {
            app_clear_graph(&staged_app);
            set_error(error_message, error_message_size, "could not connect wire", 0U);
            return false;
        }
    }

    app_update_logic(&staged_app);
    app_clear_graph(app);
    staged_app.pending_command_count = 0U;
    staged_app.drag_node = NULL;
    staged_app.active_pin = NULL;
    staged_app.wiring_active = false;
    *app = staged_app;
    rebase_app_pointers(&staged_app, app);
    return true;
}

bool circuit_file_load(AppContext *app, const char *path, char *error_message, size_t error_message_size) {
    FILE *file;
    long size;
    char *buffer;
    ParsedCircuit parsed_circuit;
    bool loaded;

    if (!app || !path) {
        set_error(error_message, error_message_size, "missing app or path", 0U);
        return false;
    }

    file = fopen(path, "rb");
    if (!file) {
        set_error(error_message, error_message_size, "could not open circuit file", 0U);
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        set_error(error_message, error_message_size, "could not seek circuit file", 0U);
        return false;
    }

    size = ftell(file);
    if (size < 0L) {
        fclose(file);
        set_error(error_message, error_message_size, "could not read circuit file size", 0U);
        return false;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        set_error(error_message, error_message_size, "could not rewind circuit file", 0U);
        return false;
    }

    buffer = (char *)calloc((size_t)size + 1U, sizeof(char));
    if (!buffer) {
        fclose(file);
        set_error(error_message, error_message_size, "out of memory", 0U);
        return false;
    }

    if (size > 0L && fread(buffer, (size_t)size, 1U, file) != 1U) {
        free(buffer);
        fclose(file);
        set_error(error_message, error_message_size, "could not read circuit file", 0U);
        return false;
    }
    fclose(file);

    loaded = parse_circuit_text(buffer, &parsed_circuit, error_message, error_message_size);
    free(buffer);
    if (!loaded) {
        return false;
    }

    return apply_parsed_circuit(app, &parsed_circuit, error_message, error_message_size);
}
