#ifndef UI_INSPECTOR_PANEL_H
#define UI_INSPECTOR_PANEL_H

#include "simulation/CircuitSimulator.h"
#include "ui/DesignTokens.h"
#include "ui/EditorSelection.h"

#include <raylib.h>

class InspectorPanel {
public:
    void render(const Rectangle& bounds,
                const DesignTokens& tokens,
                const CircuitSimulator& simulator,
                const EditorSelection& selection) const;
};

#endif // UI_INSPECTOR_PANEL_H
