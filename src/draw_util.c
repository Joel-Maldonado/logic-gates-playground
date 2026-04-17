#include "draw_util.h"

int pixel(float value) {
    return (int)value;
}

void draw_text_at(const char *text, float x, float y, int font_size, Color color) {
    DrawText(text, pixel(x), pixel(y), font_size, color);
}

void draw_line_at(float start_x, float start_y, float end_x, float end_y, Color color) {
    DrawLine(pixel(start_x), pixel(start_y), pixel(end_x), pixel(end_y), color);
}

float text_width(const char *text, int font_size) {
    int width;

    width = MeasureText(text, font_size);
    return (float)width;
}

bool begin_scissor_rect(Rectangle rect) {
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return false;
    }

    BeginScissorMode(pixel(rect.x), pixel(rect.y), pixel(rect.width), pixel(rect.height));
    return true;
}
