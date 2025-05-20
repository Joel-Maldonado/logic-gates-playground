#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>
#include <string>

namespace Config {
    // Window settings
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr int TARGET_FPS = 60;
    constexpr const char* WINDOW_TITLE = "Logic Gates Playground";

    // UI layout
    constexpr float PALETTE_WIDTH = 180.0f;
    constexpr float PALETTE_ITEM_HEIGHT = 60.0f;
    constexpr float CANVAS_PADDING = 10.0f;

    // Grid settings
    constexpr bool GRID_ENABLED = true;        // Controls whether the grid is visually displayed
    constexpr float GRID_SIZE = 25.0f;
    constexpr float GRID_LINE_THICKNESS = 1.0f;
    constexpr float GRID_OPACITY = 0.2f;  // 0.0 to 1.0

    // Gate dimensions
    constexpr float DEFAULT_GATE_WIDTH = 100.0f;
    constexpr float DEFAULT_GATE_HEIGHT = 70.0f;
    constexpr float INPUT_OUTPUT_SIZE = 45.0f;

    // Pin settings
    constexpr float PIN_CLICK_RADIUS = 7.0f;
    constexpr float PIN_HOVER_TOLERANCE = 8.0f;
    constexpr float PIN_LABEL_FONT_SIZE = 12.0f;
    constexpr float PIN_LABEL_OFFSET = 16.0f;

    // Wire settings
    constexpr float WIRE_THICKNESS_NORMAL = 2.5f;
    constexpr float WIRE_THICKNESS_SELECTED = 4.0f;
    constexpr float WIRE_THICKNESS_PREVIEW = 3.0f;
    constexpr float WIRE_HOVER_TOLERANCE = 8.0f;

    // Gate visual settings
    constexpr float GATE_LABEL_FONT_SIZE = 15.0f;
    constexpr float GATE_OUTLINE_THICKNESS = 1.5f;
    constexpr float GATE_OUTLINE_THICKNESS_SELECTED = 3.0f;

    // Animation settings
    constexpr float HOVER_ANIMATION_SPEED = 0.1f;
    constexpr float WIRE_ANIMATION_SPEED = 0.05f;

    // Drag and drop settings
    constexpr float DRAG_PREVIEW_SIZE = 50.0f; // Increased from 40.0f to 50.0f for better visibility
    constexpr float DRAG_PREVIEW_ALPHA = 0.7f;
    constexpr float DRAG_THRESHOLD = 3.0f;

    // Colors
    namespace Colors {
        // UI colors
        const Color BACKGROUND = {0x1A, 0x1A, 0x2E, 0xFF};
        const Color PALETTE = {0x2C, 0x2C, 0x2E, 0xFF};
        const Color PALETTE_ITEM = {0x36, 0x36, 0x3A, 0xFF};
        const Color PALETTE_ITEM_SELECTED = {0x4A, 0x90, 0xE2, 0xFF};
        const Color PALETTE_TEXT = {0xF0, 0xF0, 0xF0, 0xFF};
        const Color SELECTION_HIGHLIGHT = {0xFF, 0xFF, 0x00, 0xFF};
        const Color GRID_LINE = {0xCC, 0xCC, 0xCC, 0x33};        // Light gray with 0.2 opacity (0x33 is 51 in decimal, 51/255 is 0.2)

        // Wire colors
        const Color WIRE_OFF = {0x55, 0x55, 0x55, 0xFF};
        const Color WIRE_ON = {0x00, 0xFF, 0xFF, 0xFF};          // Cyan
        const Color WIRE_SELECTED = {0xFF, 0xD7, 0x00, 0xFF};    // Gold
        const Color WIRE_PREVIEW = {0xFF, 0xD7, 0x00, 0xff};     // Gold - Kept from original, seems reasonable
        const Color WIRE_INVALID = {0xFF, 0x63, 0x47, 0xff};     // Tomato Red (similar to original Orange-red)

        // Pin colors
        const Color PIN_STATE_ON = {0x00, 0xFF, 0xFF, 0xFF};     // Cyan (same as WIRE_ON)
        const Color PIN_STATE_OFF = {0x65, 0x65, 0x65, 0xFF};    // Slightly lighter gray than WIRE_OFF
        const Color PIN_HOVER = {0xAD, 0xD8, 0xE6, 0xAA};        // Light Blue with alpha (0xAA is 170, 170/255 is approx 0.66)
        const Color PIN_VALID_CONNECTION = {0x32, 0xCD, 0x32, 0xFF}; // Lime green
        const Color PIN_INVALID_CONNECTION = {0xFF, 0x63, 0x47, 0xFF}; // Tomato Red
        const Color PIN_TEXT = {0xF0, 0xF0, 0xF0, 0xFF};

        // Gate colors
        const Color GATE_FILL = {0x36, 0x45, 0x56, 0xff};        // Slate blue-gray - Default, can be overridden by specific gates
        const Color GATE_OUTLINE = {0x45, 0x45, 0x45, 0xFF};
        const Color GATE_TEXT = {0xF0, 0xF0, 0xF0, 0xFF};

        // Input/Output colors
        const Color INPUT_OFF = {0x42, 0x42, 0x42, 0xFF};
        const Color INPUT_ON = {0x00, 0xE6, 0x76, 0xFF};
        const Color OUTPUT_OFF = {0x42, 0x42, 0x42, 0xFF};
        const Color OUTPUT_ON = {0x00, 0xB0, 0xFF, 0xFF};
        const Color IO_TEXT = {0xF0, 0xF0, 0xF0, 0xFF};
        const Color IO_OUTLINE = {0x45, 0x45, 0x45, 0xff};       // Match GATE_OUTLINE

        // Gate-specific colors
        const Color AND_GATE = {0x3D, 0x84, 0xB8, 0xFF};
        const Color OR_GATE = {0x4C, 0xAF, 0x50, 0xFF};
        const Color XOR_GATE = {0x9C, 0x27, 0xB0, 0xFF};
        const Color NOT_GATE = {0xF4, 0x43, 0x36, 0xFF};
        const Color NAND_GATE = {0x5A, 0x9A, 0xCC, 0xFF};
        const Color NOR_GATE = {0x6B, 0xCF, 0x71, 0xFF};
        const Color XNOR_GATE = {0xB5, 0x50, 0xC8, 0xFF};
    }
}

#endif // CONFIG_H
