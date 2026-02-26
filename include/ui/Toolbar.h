#ifndef UI_TOOLBAR_H
#define UI_TOOLBAR_H

#include "ui/DesignTokens.h"

#include <raylib.h>

class Toolbar {
public:
    void render(const Rectangle& bounds,
                const DesignTokens& tokens,
                bool canUndo,
                bool canRedo,
                bool gridVisible,
                bool snapEnabled,
                float zoom,
                const char* modeLabel) const;
};

#endif // UI_TOOLBAR_H
