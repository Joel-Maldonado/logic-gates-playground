#ifndef UI_EDITOR_SELECTION_H
#define UI_EDITOR_SELECTION_H

#include "core/LogicGate.h"
#include "core/Wire.h"

#include <algorithm>
#include <vector>

struct EditorSelection {
    std::vector<LogicGate*> gates;
    std::vector<Wire*> wires;

    void clear() {
        gates.clear();
        wires.clear();
    }

    bool empty() const {
        return gates.empty() && wires.empty();
    }

    bool containsGate(const LogicGate* gate) const {
        return std::find(gates.begin(), gates.end(), gate) != gates.end();
    }

    bool containsWire(const Wire* wire) const {
        return std::find(wires.begin(), wires.end(), wire) != wires.end();
    }

    void addGate(LogicGate* gate) {
        if (gate && !containsGate(gate)) {
            gates.push_back(gate);
        }
    }

    void addWire(Wire* wire) {
        if (wire && !containsWire(wire)) {
            wires.push_back(wire);
        }
    }

    void toggleGate(LogicGate* gate) {
        if (!gate) {
            return;
        }

        auto it = std::find(gates.begin(), gates.end(), gate);
        if (it != gates.end()) {
            gates.erase(it);
        } else {
            gates.push_back(gate);
        }
    }

    void toggleWire(Wire* wire) {
        if (!wire) {
            return;
        }

        auto it = std::find(wires.begin(), wires.end(), wire);
        if (it != wires.end()) {
            wires.erase(it);
        } else {
            wires.push_back(wire);
        }
    }
};

#endif // UI_EDITOR_SELECTION_H
