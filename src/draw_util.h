#ifndef DRAW_UTIL_H
#define DRAW_UTIL_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "raylib.h"

typedef struct {
    uint32_t line_count;
    bool truncated;
    uint8_t _padding[3];
} WrappedTextLayout;

int pixel(float value);
void draw_text_at(const char *text, float x, float y, int font_size, Color color);
void draw_line_at(float start_x, float start_y, float end_x, float end_y, Color color);
float text_width(const char *text, int font_size);
bool text_fit_with_ellipsis(const char *text, int font_size, float max_width, char *buffer, size_t buffer_size);
WrappedTextLayout text_wrap_with_ellipsis(
    const char *text,
    int font_size,
    float max_width,
    uint32_t max_lines,
    char *buffer,
    size_t buffer_size
);
float text_wrapped_height(const char *text, int font_size, float max_width, float line_gap, uint32_t max_lines);
float draw_wrapped_text_block(
    const char *text,
    float x,
    float y,
    float max_width,
    int font_size,
    float line_gap,
    uint32_t max_lines,
    Color color
);
bool begin_scissor_rect(Rectangle rect);

#endif // DRAW_UTIL_H
