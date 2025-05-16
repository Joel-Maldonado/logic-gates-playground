#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
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
    constexpr float DEFAULT_GATE_HEIGHT = 60.0f;
    constexpr float INPUT_OUTPUT_SIZE = 50.0f;

    // Pin settings
    constexpr float PIN_CLICK_RADIUS = 8.0f;
    constexpr float PIN_HOVER_TOLERANCE = 8.0f;
    constexpr float PIN_LABEL_FONT_SIZE = 14.0f;
    constexpr float PIN_LABEL_OFFSET = 16.0f;

    // Wire settings
    constexpr float WIRE_THICKNESS_NORMAL = 3.0f;
    constexpr float WIRE_THICKNESS_SELECTED = 5.0f;
    constexpr float WIRE_THICKNESS_PREVIEW = 3.0f;
    constexpr float WIRE_HOVER_TOLERANCE = 8.0f;

    // Gate visual settings
    constexpr float GATE_LABEL_FONT_SIZE = 16.0f;
    constexpr float GATE_OUTLINE_THICKNESS = 2.0f;
    constexpr float GATE_OUTLINE_THICKNESS_SELECTED = 4.0f;

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
        const Color BACKGROUND = {0x1f, 0x29, 0x37, 0xff};       // Dark, desaturated blue-gray
        const Color PALETTE = {0x37, 0x41, 0x51, 0xff};          // Lighter gray for palette
        const Color PALETTE_ITEM = {0x4b, 0x55, 0x63, 0xff};
        const Color PALETTE_ITEM_SELECTED = {0x6b, 0x72, 0x80, 0xff};
        const Color PALETTE_TEXT = WHITE;
        const Color SELECTION_HIGHLIGHT = YELLOW;
        const Color GRID_LINE = {0x80, 0x80, 0x80, 0x33};        // Light gray with low opacity

        // Wire colors
        const Color WIRE_OFF = {0x80, 0x80, 0x80, 0xff};         // Medium gray
        const Color WIRE_ON = {0x00, 0xbf, 0xff, 0xff};          // Bright blue
        const Color WIRE_SELECTED = YELLOW;
        const Color WIRE_PREVIEW = {0xff, 0xd7, 0x00, 0xff};     // Gold
        const Color WIRE_INVALID = {0xff, 0x45, 0x00, 0xff};     // Orange-red

        // Pin colors
        const Color PIN_STATE_ON = {0x00, 0xbf, 0xff, 0xff};     // Bright blue
        const Color PIN_STATE_OFF = {0x50, 0x50, 0x50, 0xff};    // Dark gray
        const Color PIN_HOVER = {0x7f, 0xff, 0xd4, 0xff};        // Aquamarine
        const Color PIN_VALID_CONNECTION = {0x32, 0xcd, 0x32, 0xff}; // Lime green
        const Color PIN_INVALID_CONNECTION = {0xff, 0x45, 0x00, 0xff}; // Orange-red
        const Color PIN_TEXT = WHITE;

        // Gate colors
        const Color GATE_FILL = {0x36, 0x45, 0x56, 0xff};        // Slate blue-gray
        const Color GATE_OUTLINE = {0x20, 0x20, 0x20, 0xff};     // Very dark gray
        const Color GATE_TEXT = WHITE;

        // Input/Output colors
        const Color INPUT_OFF = {0x50, 0x50, 0x50, 0xff};        // Dark gray
        const Color INPUT_ON = {0x32, 0xcd, 0x32, 0xff};         // Lime green
        const Color OUTPUT_OFF = {0x50, 0x50, 0x50, 0xff};       // Dark gray
        const Color OUTPUT_ON = {0xff, 0x45, 0x00, 0xff};        // Orange-red
        const Color IO_TEXT = WHITE;
        const Color IO_OUTLINE = {0x20, 0x20, 0x20, 0xff};       // Very dark gray

        // Gate-specific colors
        const Color AND_GATE = {0x41, 0x69, 0xe1, 0xff};         // Royal blue
        const Color OR_GATE = {0x9a, 0xcd, 0x32, 0xff};          // Yellow-green
        const Color XOR_GATE = {0xff, 0x8c, 0x00, 0xff};         // Dark orange
        const Color NOT_GATE = {0xdc, 0x14, 0x3c, 0xff};         // Crimson
        const Color NAND_GATE = {0x48, 0x3d, 0x8b, 0xff};        // Dark slate blue
        const Color NOR_GATE = {0x2e, 0x8b, 0x57, 0xff};         // Sea green
        const Color XNOR_GATE = {0xda, 0x70, 0xd6, 0xff};        // Orchid
    }
}

#endif // CONFIG_H
