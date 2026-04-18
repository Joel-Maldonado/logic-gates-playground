#include "draw_util.h"
#include <stdio.h>
#include <string.h>

#define TEXT_WRAP_BUFFER_MAX 1024

static void copy_text(char *buffer, size_t buffer_size, const char *text) {
    if (!buffer || buffer_size == 0U) {
        return;
    }

    if (!text) {
        buffer[0] = '\0';
        return;
    }

    snprintf(buffer, buffer_size, "%s", text);
}

static void append_text(char *buffer, size_t buffer_size, size_t *length, const char *text) {
    size_t available;
    int written;

    if (!buffer || !length || !text || *length >= buffer_size) {
        return;
    }

    available = buffer_size - *length;
    written = snprintf(buffer + *length, available, "%s", text);
    if (written <= 0) {
        return;
    }

    if ((size_t)written >= available) {
        *length = buffer_size - 1U;
        return;
    }

    *length += (size_t)written;
}

static void truncate_last_wrapped_line(char *buffer, size_t buffer_size, int font_size, float max_width) {
    char fitted[TEXT_WRAP_BUFFER_MAX];
    char last_line[TEXT_WRAP_BUFFER_MAX];
    char candidate[TEXT_WRAP_BUFFER_MAX];
    char *last_line_start;
    size_t prefix_length;

    if (!buffer || buffer_size == 0U || buffer[0] == '\0') {
        return;
    }

    last_line_start = strrchr(buffer, '\n');
    if (last_line_start) {
        last_line_start++;
    } else {
        last_line_start = buffer;
    }

    snprintf(last_line, sizeof(last_line), "%s", last_line_start);
    snprintf(candidate, sizeof(candidate), "%s...", last_line);
    text_fit_with_ellipsis(candidate, font_size, max_width, fitted, sizeof(fitted));

    prefix_length = (size_t)(last_line_start - buffer);
    buffer[prefix_length] = '\0';
    snprintf(buffer + prefix_length, buffer_size - prefix_length, "%s", fitted);
}

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
    if (width == 0 && text && text[0] != '\0') {
        size_t text_length;

        text_length = strlen(text);
        width = (int)((float)text_length * ((float)font_size * 0.6f));
        if (width < 1) {
            width = 1;
        }
    }
    return (float)width;
}

bool text_fit_with_ellipsis(const char *text, int font_size, float max_width, char *buffer, size_t buffer_size) {
    static const char ellipsis[] = "...";
    size_t length;
    size_t prefix_length;
    char candidate[TEXT_WRAP_BUFFER_MAX];

    if (!buffer || buffer_size == 0U) {
        return false;
    }

    buffer[0] = '\0';
    if (!text) {
        return false;
    }

    if (max_width <= 0.0f) {
        return false;
    }

    if (text_width(text, font_size) <= max_width) {
        copy_text(buffer, buffer_size, text);
        return true;
    }

    if (text_width(ellipsis, font_size) > max_width) {
        return false;
    }

    length = strlen(text);
    for (prefix_length = length; prefix_length > 0U; prefix_length--) {
        snprintf(candidate, sizeof(candidate), "%.*s%s", (int)prefix_length, text, ellipsis);
        if (text_width(candidate, font_size) <= max_width) {
            copy_text(buffer, buffer_size, candidate);
            return true;
        }
    }

    copy_text(buffer, buffer_size, ellipsis);
    return true;
}

WrappedTextLayout text_wrap_with_ellipsis(
    const char *text,
    int font_size,
    float max_width,
    uint32_t max_lines,
    char *buffer,
    size_t buffer_size
) {
    WrappedTextLayout layout;
    const char *cursor;
    char line[TEXT_WRAP_BUFFER_MAX];
    size_t output_length;

    memset(&layout, 0, sizeof(layout));
    if (!buffer || buffer_size == 0U) {
        return layout;
    }

    buffer[0] = '\0';
    if (!text || text[0] == '\0' || max_width <= 0.0f || max_lines == 0U) {
        return layout;
    }

    cursor = text;
    line[0] = '\0';
    output_length = 0U;

    while (*cursor != '\0') {
        char word[TEXT_WRAP_BUFFER_MAX];
        char candidate[TEXT_WRAP_BUFFER_MAX];
        size_t word_length;

        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r') {
            cursor++;
        }

        if (*cursor == '\n') {
            if (line[0] != '\0') {
                if (layout.line_count > 0U) {
                    append_text(buffer, buffer_size, &output_length, "\n");
                }
                append_text(buffer, buffer_size, &output_length, line);
                layout.line_count++;
                line[0] = '\0';
                if (layout.line_count >= max_lines && cursor[1] != '\0') {
                    layout.truncated = true;
                    break;
                }
            }
            cursor++;
            continue;
        }

        if (*cursor == '\0') {
            break;
        }

        word_length = 0U;
        while (
            cursor[word_length] != '\0' &&
            cursor[word_length] != ' ' &&
            cursor[word_length] != '\t' &&
            cursor[word_length] != '\r' &&
            cursor[word_length] != '\n' &&
            word_length + 1U < sizeof(word)
        ) {
            word_length++;
        }
        memcpy(word, cursor, word_length);
        word[word_length] = '\0';
        cursor += word_length;

        if (text_width(word, font_size) > max_width) {
            char fitted_word[TEXT_WRAP_BUFFER_MAX];

            text_fit_with_ellipsis(word, font_size, max_width, fitted_word, sizeof(fitted_word));
            if (line[0] != '\0') {
                if (layout.line_count > 0U) {
                    append_text(buffer, buffer_size, &output_length, "\n");
                }
                append_text(buffer, buffer_size, &output_length, line);
                layout.line_count++;
                line[0] = '\0';
                if (layout.line_count >= max_lines) {
                    layout.truncated = true;
                    break;
                }
            }

            if (layout.line_count > 0U) {
                append_text(buffer, buffer_size, &output_length, "\n");
            }
            append_text(buffer, buffer_size, &output_length, fitted_word);
            layout.line_count++;
            if (layout.line_count >= max_lines && *cursor != '\0') {
                layout.truncated = true;
                break;
            }
            continue;
        }

        if (line[0] == '\0') {
            copy_text(line, sizeof(line), word);
            continue;
        }

        snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        if (text_width(candidate, font_size) <= max_width) {
            copy_text(line, sizeof(line), candidate);
            continue;
        }

        if (layout.line_count > 0U) {
            append_text(buffer, buffer_size, &output_length, "\n");
        }
        append_text(buffer, buffer_size, &output_length, line);
        layout.line_count++;
        if (layout.line_count >= max_lines) {
            layout.truncated = true;
            break;
        }

        copy_text(line, sizeof(line), word);
    }

    if (!layout.truncated && line[0] != '\0') {
        if (layout.line_count > 0U) {
            append_text(buffer, buffer_size, &output_length, "\n");
        }
        append_text(buffer, buffer_size, &output_length, line);
        layout.line_count++;
    }

    if (layout.truncated) {
        truncate_last_wrapped_line(buffer, buffer_size, font_size, max_width);
    }

    return layout;
}

float text_wrapped_height(const char *text, int font_size, float max_width, float line_gap, uint32_t max_lines) {
    WrappedTextLayout layout;
    char wrapped[TEXT_WRAP_BUFFER_MAX];

    layout = text_wrap_with_ellipsis(text, font_size, max_width, max_lines, wrapped, sizeof(wrapped));
    if (layout.line_count == 0U) {
        return 0.0f;
    }

    return ((float)layout.line_count * (float)font_size) + ((float)(layout.line_count - 1U) * line_gap);
}

float draw_wrapped_text_block(
    const char *text,
    float x,
    float y,
    float max_width,
    int font_size,
    float line_gap,
    uint32_t max_lines,
    Color color
) {
    WrappedTextLayout layout;
    char wrapped[TEXT_WRAP_BUFFER_MAX];
    char *line;
    char *next_line;

    layout = text_wrap_with_ellipsis(text, font_size, max_width, max_lines, wrapped, sizeof(wrapped));
    if (layout.line_count == 0U) {
        return y;
    }

    line = wrapped;
    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
        }

        draw_text_at(line, x, y, font_size, color);
        y += (float)font_size + line_gap;

        if (!next_line) {
            break;
        }
        line = next_line + 1;
    }

    return y - line_gap;
}

bool begin_scissor_rect(Rectangle rect) {
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return false;
    }

    BeginScissorMode(pixel(rect.x), pixel(rect.y), pixel(rect.width), pixel(rect.height));
    return true;
}
