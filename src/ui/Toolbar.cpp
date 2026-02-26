#include "ui/Toolbar.h"

#include <raylib.h>

void Toolbar::render(const Rectangle& bounds,
                     const DesignTokens& tokens,
                     bool canUndo,
                     bool canRedo,
                     bool gridVisible,
                     bool snapEnabled,
                     float zoom,
                     const char* modeLabel) const {
    DrawRectangleRec(bounds, tokens.colors.panelBackground);
    DrawLineEx({bounds.x, bounds.y + bounds.height - 1.0f},
               {bounds.x + bounds.width, bounds.y + bounds.height - 1.0f},
               1.0f,
               tokens.colors.panelBorder);

    DrawTextEx(tokens.typography.ui,
               "Logic Gates Playground",
               {bounds.x + 12.0f, bounds.y + 12.0f},
               tokens.typography.bodySize,
               1.0f,
               tokens.colors.textPrimary);

    const char* undoText = canUndo ? "Undo" : "Undo (disabled)";
    const char* redoText = canRedo ? "Redo" : "Redo (disabled)";

    DrawTextEx(tokens.typography.mono,
               undoText,
               {bounds.x + 300.0f, bounds.y + 14.0f},
               tokens.typography.smallSize,
               1.0f,
               canUndo ? tokens.colors.textPrimary : tokens.colors.textMuted);

    DrawTextEx(tokens.typography.mono,
               redoText,
               {bounds.x + 430.0f, bounds.y + 14.0f},
               tokens.typography.smallSize,
               1.0f,
               canRedo ? tokens.colors.textPrimary : tokens.colors.textMuted);

    char rightInfo[256];
    snprintf(rightInfo,
             sizeof(rightInfo),
             "%s  |  Grid:%s  |  Snap:%s  |  Zoom:%d%%",
             modeLabel ? modeLabel : "Idle",
             gridVisible ? "On" : "Off",
             snapEnabled ? "On" : "Off",
             static_cast<int>(zoom * 100.0f));

    Vector2 size = MeasureTextEx(tokens.typography.mono, rightInfo, tokens.typography.smallSize, 1.0f);
    DrawTextEx(tokens.typography.mono,
               rightInfo,
               {bounds.x + bounds.width - size.x - 16.0f, bounds.y + 14.0f},
               tokens.typography.smallSize,
               1.0f,
               tokens.colors.textMuted);
}
