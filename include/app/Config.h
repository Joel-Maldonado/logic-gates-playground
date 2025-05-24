#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>
#include <string>

namespace Config {
    // Window configuration
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr int TARGET_FPS = 60;
    constexpr const char* WINDOW_TITLE = "Logic Gates Playground";

    // UI layout constants
    constexpr float PALETTE_WIDTH = 180.0f;
    constexpr float PALETTE_ITEM_HEIGHT = 60.0f;
    constexpr float CANVAS_PADDING = 10.0f;

    // Grid configuration
    constexpr bool GRID_ENABLED = true;
    constexpr float GRID_SIZE = 25.0f;
    constexpr float GRID_LINE_THICKNESS = 1.0f;
    constexpr float GRID_OPACITY = 0.2f;

    // Default component sizes
    constexpr float DEFAULT_GATE_WIDTH = 100.0f;
    constexpr float DEFAULT_GATE_HEIGHT = 60.0f;
    constexpr float INPUT_OUTPUT_SIZE = 50.0f;

    // Pin settings
    constexpr float PIN_CLICK_RADIUS = 8.0f;
    constexpr float PIN_HOVER_TOLERANCE = 8.0f;
    constexpr float PIN_LABEL_FONT_SIZE = 16.0f;
    constexpr float PIN_LABEL_OFFSET = 16.0f;
    constexpr float PIN_RADIUS = 5.0f;

    // Wire settings
    constexpr float WIRE_THICKNESS_NORMAL = 3.0f;
    constexpr float WIRE_THICKNESS_SELECTED = 5.0f;
    constexpr float WIRE_THICKNESS_PREVIEW = 3.0f;
    constexpr float WIRE_HOVER_TOLERANCE = 8.0f;
    constexpr float WIRE_HORIZONTAL_OFFSET = 10.0f;

    // Gate visual settings
    constexpr float GATE_LABEL_FONT_SIZE = 18.0f;
    constexpr float GATE_OUTLINE_THICKNESS = 2.0f;
    constexpr float GATE_OUTLINE_THICKNESS_SELECTED = 3.0f;
    constexpr float GATE_TEXT_SIZE = 20.0f;
    constexpr float GATE_TEXT_OFFSET = 5.0f;

    // Component visual settings
    constexpr float CONNECTOR_PIN_RADIUS = 5.0f;
    constexpr float LABEL_FONT_SIZE = 14.0f;
    constexpr float LABEL_SPACING = 3.0f;

    // Gate rendering constants
    constexpr float GATE_WIDTH_RATIO = 0.85f;
    constexpr float GATE_CURVE_DEPTH_RATIO = 0.15f;
    constexpr float GATE_XOR_CURVE_DEPTH_RATIO = 0.12f;
    constexpr float GATE_NOT_WIDTH_RATIO = 0.7f;
    constexpr float GATE_TRIANGLE_ASPECT_RATIO = 0.866f;
    constexpr float GATE_OR_WIDTH_RATIO = 0.8f;
    constexpr float GATE_XOR_WIDTH_RATIO = 0.8f;
    constexpr float INVERSION_BUBBLE_RADIUS = 5.0f;
    constexpr int GATE_CURVE_SEGMENTS = 20;
    constexpr float GATE_SHADOW_GRADIENT_LAYERS = 10.0f;
    constexpr float XOR_LINE_OFFSET = 4.0f;
    constexpr float GATE_FONT_SIZE_RATIO = 0.9f;

    // Animation settings
    constexpr float HOVER_ANIMATION_SPEED = 0.1f;
    constexpr float WIRE_ANIMATION_SPEED = 0.05f;

    // Drag and drop settings
    constexpr float DRAG_PREVIEW_SIZE = 50.0f; // Increased from 40.0f to 50.0f for better visibility
    constexpr float DRAG_PREVIEW_ALPHA = 0.7f;
    constexpr float DRAG_THRESHOLD = 3.0f;

    // Animation and timing settings
    constexpr float ANIMATION_SPEED = 4.0f;
    constexpr float HOVER_TRANSITION_SPEED = 8.0f;
    constexpr float PULSE_SPEED = 2.0f;
    constexpr float GLOW_INTENSITY = 0.3f;
    constexpr float SHADOW_OFFSET = 2.0f;
    constexpr float CORNER_RADIUS = 8.0f;

    // Colors - Modern sleek theme with subtle sophistication
    namespace Colors {
        // Base palette - sophisticated dark theme
        const Color BACKGROUND = {0x12, 0x12, 0x16, 0xff};       // Deep charcoal
        const Color SURFACE = {0x1e, 0x1e, 0x24, 0xff};          // Dark surface
        const Color SURFACE_VARIANT = {0x28, 0x28, 0x30, 0xff};  // Card surfaces

        // Accent colors - muted and sophisticated
        const Color PRIMARY = {0x4a, 0x9e, 0xff, 0xff};          // Soft blue
        const Color PRIMARY_VARIANT = {0x3a, 0x7e, 0xd9, 0xff};  // Darker soft blue
        const Color SECONDARY = {0x8b, 0x5a, 0xff, 0xff};        // Soft purple
        const Color TERTIARY = {0xff, 0x8a, 0x50, 0xff};         // Soft orange

        // UI colors
        const Color PALETTE = {0x1e, 0x1e, 0x24, 0xff};          // Surface color
        const Color PALETTE_ITEM = {0x28, 0x28, 0x30, 0xff};     // Card color
        const Color PALETTE_ITEM_HOVER = {0x32, 0x32, 0x3c, 0xff}; // Hover state
        const Color PALETTE_ITEM_SELECTED = {0x3c, 0x3c, 0x48, 0xff}; // Selected state
        const Color PALETTE_TEXT = {0xe0, 0xe0, 0xe6, 0xff};     // Soft white text
        const Color SELECTION_HIGHLIGHT = {0xff, 0xd7, 0x00, 0xff}; // Yellow highlight
        const Color GRID_LINE = {0x38, 0x38, 0x42, 0x30};        // Very subtle grid lines
        const Color GRID_DOT = {0x42, 0x42, 0x4e, 0x40};         // Subtle grid dots

        // Wire colors - subtle and elegant
        const Color WIRE_OFF = {0x5a, 0x5a, 0x66, 0xff};         // Muted gray
        const Color WIRE_ON = {0x4a, 0x9e, 0xff, 0xff};          // Soft blue active
        const Color WIRE_SELECTED = {0xff, 0x8a, 0x50, 0xff};    // Soft orange selection
        const Color WIRE_PREVIEW = {0x8b, 0x5a, 0xff, 0xff};     // Soft purple preview
        const Color WIRE_INVALID = {0xff, 0x6b, 0x6b, 0xff};     // Soft red error

        // Pin colors - sophisticated and clear
        const Color PIN_STATE_ON = {0x4a, 0x9e, 0xff, 0xff};     // Soft blue active
        const Color PIN_STATE_OFF = {0x5a, 0x5a, 0x66, 0xff};    // Muted gray inactive
        const Color PIN_HOVER = {0x8b, 0x5a, 0xff, 0xff};        // Soft purple hover
        const Color PIN_VALID_CONNECTION = {0x5d, 0xc9, 0x60, 0xff}; // Soft green valid
        const Color PIN_INVALID_CONNECTION = {0xff, 0x6b, 0x6b, 0xff}; // Soft red invalid
        const Color PIN_TEXT = {0xe0, 0xe0, 0xe6, 0xff};         // Soft white text

        // Gate colors - clean modern style
        const Color GATE_FILL = {0x28, 0x28, 0x30, 0xff};        // Base surface
        const Color GATE_FILL_LIGHT = {0x32, 0x32, 0x3c, 0xff};  // Highlight
        const Color GATE_OUTLINE = {0x5a, 0x5a, 0x66, 0xff};     // Subtle outline
        const Color GATE_SHADOW = {0x00, 0x00, 0x00, 0x20};      // Subtle drop shadow
        const Color GATE_TEXT = {0xe0, 0xe0, 0xe6, 0xff};        // Soft white text

        // Input/Output colors - clear but not harsh
        const Color INPUT_OFF = {0x5a, 0x5a, 0x66, 0xff};        // Muted gray off
        const Color INPUT_ON = {0x5d, 0xc9, 0x60, 0xff};         // Soft green active
        const Color OUTPUT_OFF = {0x5a, 0x5a, 0x66, 0xff};       // Muted gray off
        const Color OUTPUT_ON = {0xff, 0x8a, 0x50, 0xff};        // Soft orange active
        const Color IO_TEXT = {0xe0, 0xe0, 0xe6, 0xff};          // Soft white text
        const Color IO_OUTLINE = {0x5a, 0x5a, 0x66, 0xff};       // Subtle outline

        const Color AND_GATE = {0x4a, 0x9e, 0xff, 0xff};
        const Color AND_GATE_LIGHT = {0x6b, 0xb6, 0xff, 0xff};
        const Color OR_GATE = {0x5d, 0xc9, 0x60, 0xff};
        const Color OR_GATE_LIGHT = {0x7d, 0xd9, 0x80, 0xff};
        const Color XOR_GATE = {0xff, 0x8a, 0x50, 0xff};
        const Color XOR_GATE_LIGHT = {0xff, 0xa6, 0x70, 0xff};
        const Color NOT_GATE = {0x8b, 0x5a, 0xff, 0xff};
        const Color NOT_GATE_LIGHT = {0xa5, 0x7a, 0xff, 0xff};
    }
}

#endif // CONFIG_H
