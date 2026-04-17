#ifndef DRAW_UTIL_H
#define DRAW_UTIL_H

#include <stdbool.h>
#include "raylib.h"

int pixel(float value);
void draw_text_at(const char *text, float x, float y, int font_size, Color color);
void draw_line_at(float start_x, float start_y, float end_x, float end_y, Color color);
float text_width(const char *text, int font_size);
bool begin_scissor_rect(Rectangle rect);

#endif // DRAW_UTIL_H
