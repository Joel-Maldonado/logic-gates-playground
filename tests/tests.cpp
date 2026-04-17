#include "core/DerivedGates.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "simulation/CircuitSimulator.h"
#include "ui/CommandStack.h"
#include "ui/EditorSelection.h"
#include "ui/GateGeometry.h"
#include "ui/InteractionHelpers.h"
#include "ui/commands/DuplicateSelectionCommand.h"
#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testTruthTables() {
    const bool values[] = {false, true};

    for (bool a : values) {
        for (bool b : values) {
            AndGate andGate("and", {0.0f, 0.0f}, 100.0f, 60.0f);
            andGate.setInputState(0, a);
            andGate.setInputState(1, b);
            andGate.update();
            expect(andGate.getOutputState(0) == (a && b), "AND truth table mismatch");

            OrGate orGate("or", {0.0f, 0.0f}, 100.0f, 60.0f);
            orGate.setInputState(0, a);
            orGate.setInputState(1, b);
            orGate.update();
            expect(orGate.getOutputState(0) == (a || b), "OR truth table mismatch");

            XorGate xorGate("xor", {0.0f, 0.0f}, 100.0f, 60.0f);
            xorGate.setInputState(0, a);
            xorGate.setInputState(1, b);
            xorGate.update();
            expect(xorGate.getOutputState(0) == (a != b), "XOR truth table mismatch");
        }

        NotGate notGate("not", {0.0f, 0.0f}, 100.0f, 60.0f);
        notGate.setInputState(0, a);
        notGate.update();
        expect(notGate.getOutputState(0) == (!a), "NOT truth table mismatch");
    }
}

bool runDepthChainOutput(int depth, bool shuffledOrder) {
    CircuitSimulator simulator;

    auto* input = static_cast<InputSource*>(simulator.addGate(
        std::make_unique<InputSource>("in", Vector2{0.0f, 0.0f}, Vector2{50.0f, 50.0f}, "IN")));
    expect(input != nullptr, "Failed to add input source");

    std::vector<NotGate*> notGates(static_cast<size_t>(depth), nullptr);
    std::vector<int> indices(static_cast<size_t>(depth));
    std::iota(indices.begin(), indices.end(), 0);

    if (shuffledOrder) {
        std::mt19937 rng(12345);
        std::shuffle(indices.begin(), indices.end(), rng);
    }

    for (int idx : indices) {
        std::string id = "n" + std::to_string(idx);
        notGates[static_cast<size_t>(idx)] = static_cast<NotGate*>(simulator.addGate(
            std::make_unique<NotGate>(id, Vector2{100.0f + idx * 80.0f, 0.0f}, 100.0f, 60.0f)));
        expect(notGates[static_cast<size_t>(idx)] != nullptr, "Failed to add NOT gate");
    }

    auto* output = static_cast<OutputSink*>(simulator.addGate(
        std::make_unique<OutputSink>("out", Vector2{900.0f, 0.0f}, 25.0f, "OUT")));
    expect(output != nullptr, "Failed to add output sink");

    expect(simulator.createWire(input->getOutputPin(0), notGates.front()->getInputPin(0)) != nullptr,
           "Failed to connect input to first NOT");

    for (int i = 0; i < depth - 1; ++i) {
        expect(simulator.createWire(notGates[static_cast<size_t>(i)]->getOutputPin(0),
                                    notGates[static_cast<size_t>(i + 1)]->getInputPin(0)) != nullptr,
               "Failed to connect NOT chain");
    }

    expect(simulator.createWire(notGates.back()->getOutputPin(0), output->getInputPin(0)) != nullptr,
           "Failed to connect final NOT to output");

    input->setState(true);
    CircuitSimulator::SimulationStats stats = simulator.update();

    expect(stats.stable, "Depth chain should stabilize");
    expect(!stats.oscillating, "Depth chain should not oscillate");
    expect(stats.passes >= 1 && stats.passes <= 64, "Pass count out of expected bounds");

    bool expectedOutput = (depth % 2 == 0);
    expect(output->isActive() == expectedOutput, "Depth chain output mismatch");

    return output->isActive();
}

float pointSegmentDistance(Vector2 p, Vector2 a, Vector2 b) {
    const Vector2 ab = Vector2Subtract(b, a);
    const float lenSq = Vector2LengthSqr(ab);
    if (lenSq < 1e-6f) {
        return Vector2Distance(p, a);
    }

    const float t = std::clamp(Vector2DotProduct(Vector2Subtract(p, a), ab) / lenSq, 0.0f, 1.0f);
    const Vector2 projection = Vector2Add(a, Vector2Scale(ab, t));
    return Vector2Distance(p, projection);
}

float minDistanceToStroke(const std::vector<Vector2>& stroke, Vector2 point) {
    if (stroke.size() < 2) {
        return 1e9f;
    }

    float minDistance = 1e9f;
    for (size_t i = 0; i < stroke.size(); ++i) {
        const Vector2 a = stroke[i];
        const Vector2 b = stroke[(i + 1) % stroke.size()];
        minDistance = std::min(minDistance, pointSegmentDistance(point, a, b));
    }

    return minDistance;
}

void testDepthPropagationAndOrderInvariance() {
    bool ordered = runDepthChainOutput(8, false);
    bool shuffled = runDepthChainOutput(8, true);
    expect(ordered == shuffled, "Output should be invariant to insertion order");
}

void testFanoutPropagation() {
    CircuitSimulator simulator;

    auto* input = static_cast<InputSource*>(simulator.addGate(
        std::make_unique<InputSource>("in", Vector2{0.0f, 0.0f}, Vector2{50.0f, 50.0f}, "IN")));
    auto* notA = static_cast<NotGate*>(simulator.addGate(
        std::make_unique<NotGate>("na", Vector2{100.0f, -40.0f}, 100.0f, 60.0f)));
    auto* notB = static_cast<NotGate*>(simulator.addGate(
        std::make_unique<NotGate>("nb", Vector2{100.0f, 40.0f}, 100.0f, 60.0f)));
    auto* orGate = static_cast<OrGate*>(simulator.addGate(
        std::make_unique<OrGate>("or", Vector2{220.0f, 0.0f}, 100.0f, 60.0f)));
    auto* output = static_cast<OutputSink*>(simulator.addGate(
        std::make_unique<OutputSink>("out", Vector2{360.0f, 0.0f}, 25.0f, "OUT")));

    expect(input && notA && notB && orGate && output, "Failed to add fanout test gates");

    expect(simulator.createWire(input->getOutputPin(0), notA->getInputPin(0)) != nullptr,
           "Failed to connect input->notA");
    expect(simulator.createWire(input->getOutputPin(0), notB->getInputPin(0)) != nullptr,
           "Failed to connect input->notB");
    expect(simulator.createWire(notA->getOutputPin(0), orGate->getInputPin(0)) != nullptr,
           "Failed to connect notA->or");
    expect(simulator.createWire(notB->getOutputPin(0), orGate->getInputPin(1)) != nullptr,
           "Failed to connect notB->or");
    expect(simulator.createWire(orGate->getOutputPin(0), output->getInputPin(0)) != nullptr,
           "Failed to connect or->output");

    input->setState(true);
    expect(simulator.update().stable, "Fanout circuit should stabilize for input=1");
    expect(output->isActive() == false, "Fanout output mismatch for input=1");

    input->setState(false);
    expect(simulator.update().stable, "Fanout circuit should stabilize for input=0");
    expect(output->isActive() == true, "Fanout output mismatch for input=0");
}

void testOscillationDetection() {
    CircuitSimulator simulator;

    auto* notGate = static_cast<NotGate*>(simulator.addGate(
        std::make_unique<NotGate>("self", Vector2{100.0f, 100.0f}, 100.0f, 60.0f)));
    expect(notGate != nullptr, "Failed to add self-loop NOT gate");

    expect(simulator.createWire(notGate->getOutputPin(0), notGate->getInputPin(0)) != nullptr,
           "Failed to create self-loop wire");

    CircuitSimulator::SimulationStats stats = simulator.update();
    expect(!stats.stable, "Self-loop NOT should not be stable");
    expect(stats.oscillating, "Self-loop NOT should be flagged as oscillating");
    expect(stats.passes == 64, "Oscillation should stop at max pass guard");
}

void testGateDeletionCleansWires() {
    CircuitSimulator simulator;

    auto* input = static_cast<InputSource*>(simulator.addGate(
        std::make_unique<InputSource>("in", Vector2{0.0f, 0.0f}, Vector2{50.0f, 50.0f}, "IN")));
    auto* notGate = static_cast<NotGate*>(simulator.addGate(
        std::make_unique<NotGate>("not", Vector2{120.0f, 0.0f}, 100.0f, 60.0f)));
    auto* output = static_cast<OutputSink*>(simulator.addGate(
        std::make_unique<OutputSink>("out", Vector2{260.0f, 0.0f}, 25.0f, "OUT")));

    expect(input && notGate && output, "Failed to add deletion test gates");

    expect(simulator.createWire(input->getOutputPin(0), notGate->getInputPin(0)) != nullptr,
           "Failed to create wire input->not");
    expect(simulator.createWire(notGate->getOutputPin(0), output->getInputPin(0)) != nullptr,
           "Failed to create wire not->output");

    expect(simulator.getWires().size() == 2, "Expected 2 wires before gate deletion");
    expect(simulator.removeGate(notGate), "Expected removeGate to succeed");
    expect(simulator.getWires().empty(), "All connected wires should be removed with gate deletion");

    for (const auto& gate : simulator.getGates()) {
        expect(gate->getAssociatedWires().empty(), "Remaining gate should have no dangling associated wires");
    }
}

void testInteractionHelpers() {
    Vector2 start = {0.0f, 0.0f};
    Vector2 nearPoint = {2.0f, 2.0f};
    Vector2 farPoint = {10.0f, 0.0f};

    expect(InteractionHelpers::isClickWithinThreshold(start, nearPoint, 3.0f),
           "Near point should be treated as click");
    expect(!InteractionHelpers::isClickWithinThreshold(start, farPoint, 3.0f),
           "Far point should not be treated as click");
    expect(InteractionHelpers::exceedsDragThreshold(start, farPoint, 3.0f),
           "Far point should exceed drag threshold");

    InteractionHelpers::DragAxis horizontal =
        InteractionHelpers::determineDominantAxis(Vector2{0.0f, 0.0f}, Vector2{6.0f, 1.0f});
    InteractionHelpers::DragAxis vertical =
        InteractionHelpers::determineDominantAxis(Vector2{0.0f, 0.0f}, Vector2{1.0f, 6.0f});

    expect(horizontal == InteractionHelpers::DragAxis::HORIZONTAL,
           "Expected horizontal dominant axis");
    expect(vertical == InteractionHelpers::DragAxis::VERTICAL,
           "Expected vertical dominant axis");

    Vector2 lockedHorizontal = InteractionHelpers::applyAxisLock(
        Vector2{20.0f, 25.0f}, Vector2{5.0f, 5.0f}, InteractionHelpers::DragAxis::HORIZONTAL);
    Vector2 lockedVertical = InteractionHelpers::applyAxisLock(
        Vector2{20.0f, 25.0f}, Vector2{5.0f, 5.0f}, InteractionHelpers::DragAxis::VERTICAL);

    expect(lockedHorizontal.y == 5.0f, "Horizontal lock should keep anchor Y");
    expect(lockedVertical.x == 5.0f, "Vertical lock should keep anchor X");
}

void testGateGeometryPinAnchorParity() {
    std::vector<std::unique_ptr<LogicGate>> gates;
    gates.push_back(std::make_unique<AndGate>("and", Vector2{10.0f, 20.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<OrGate>("or", Vector2{25.0f, 80.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<XorGate>("xor", Vector2{25.0f, 140.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<NotGate>("not", Vector2{30.0f, 210.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<InputSource>("in", Vector2{20.0f, 290.0f}, Vector2{50.0f, 50.0f}, "IN"));
    gates.push_back(std::make_unique<OutputSink>("out", Vector2{120.0f, 290.0f}, 25.0f, "OUT"));

    for (const auto& gate : gates) {
        const Rectangle bounds = gate->getBounds();
        const std::vector<Vector2> anchors = GateGeometry::pinAnchors(gate->getKind(), bounds);

        std::vector<Vector2> actualPins;
        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
            actualPins.push_back(gate->getInputPin(i)->getAbsolutePosition());
        }
        for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
            actualPins.push_back(gate->getOutputPin(i)->getAbsolutePosition());
        }

        expect(anchors.size() == actualPins.size(), "Pin anchor count mismatch");

        for (size_t i = 0; i < anchors.size(); ++i) {
            const float distance = Vector2Distance(anchors[i], actualPins[i]);
            expect(distance < 0.001f, "Pin anchor does not match gate pin position");
        }
    }
}

void testGateGeometryHitTest() {
    const Rectangle notBounds = {100.0f, 100.0f, 100.0f, 60.0f};
    expect(GateGeometry::hitTestBody(GateKind::NOT_GATE, notBounds, {140.0f, 130.0f}),
           "NOT gate center should hit");
    expect(!GateGeometry::hitTestBody(GateKind::NOT_GATE, notBounds, {90.0f, 130.0f}),
           "Point outside NOT gate should miss");

    const Rectangle outBounds = {260.0f, 120.0f, 50.0f, 50.0f};
    expect(GateGeometry::hitTestBody(GateKind::OUTPUT_SINK, outBounds, {285.0f, 145.0f}),
           "Output center should hit");
    expect(!GateGeometry::hitTestBody(GateKind::OUTPUT_SINK, outBounds, {320.0f, 145.0f}),
           "Point outside output circle should miss");
}

void testOrXorGeometryDifferentiation() {
    const Rectangle bounds = {20.0f, 40.0f, 100.0f, 60.0f};
    const GateShapeData orShape = GateGeometry::buildShape(GateKind::OR_GATE, bounds);
    const GateShapeData xorShape = GateGeometry::buildShape(GateKind::XOR_GATE, bounds);

    expect(orShape.accentStrokes.empty(), "OR should not have additional accent strokes");
    expect(xorShape.accentStrokes.size() == 1, "XOR should have exactly one rear accent stroke");
    expect(!xorShape.accentStrokes.front().empty(), "XOR rear accent stroke should contain points");
}

void testCurvedGatePinBoundaryContact() {
    std::vector<std::unique_ptr<LogicGate>> gates;
    gates.push_back(std::make_unique<OrGate>("or", Vector2{25.0f, 80.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<XorGate>("xor", Vector2{25.0f, 140.0f}, 100.0f, 60.0f));
    gates.push_back(std::make_unique<NotGate>("not", Vector2{30.0f, 210.0f}, 100.0f, 60.0f));

    for (const auto& gate : gates) {
        const GateShapeData shape = GateGeometry::buildShape(gate->getKind(), gate->getBounds());

        std::vector<const GatePin*> pins;
        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
            pins.push_back(gate->getInputPin(i));
        }
        for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
            pins.push_back(gate->getOutputPin(i));
        }

        for (const GatePin* pin : pins) {
            const Vector2 position = pin->getAbsolutePosition();
            float distance = minDistanceToStroke(shape.strokePath, position);
            if (shape.hasBubble) {
                distance = std::min(distance, fabsf(Vector2Distance(position, shape.bubbleCenter) - shape.bubbleRadius));
            }
            expect(distance < 0.25f, "Curved gate pin should contact its boundary");
        }
    }
}

void testSnapToGridHelper() {
    const Vector2 snappedA = InteractionHelpers::snapToGrid({37.0f, 62.0f}, 25.0f);
    expect(fabsf(snappedA.x - 25.0f) < 0.001f && fabsf(snappedA.y - 50.0f) < 0.001f,
           "snapToGrid should round to nearest grid intersection");

    const Vector2 snappedB = InteractionHelpers::snapToGrid({63.0f, 63.0f}, 25.0f);
    expect(fabsf(snappedB.x - 75.0f) < 0.001f && fabsf(snappedB.y - 75.0f) < 0.001f,
           "snapToGrid should round midpoint-away values correctly");

    const Vector2 snappedC = InteractionHelpers::snapToGrid({-13.0f, -38.0f}, 25.0f);
    expect(fabsf(snappedC.x + 25.0f) < 0.001f && fabsf(snappedC.y + 50.0f) < 0.001f,
           "snapToGrid should support negative coordinates");

    const Vector2 unchanged = InteractionHelpers::snapToGrid({12.0f, 18.0f}, 0.0f);
    expect(fabsf(unchanged.x - 12.0f) < 0.001f && fabsf(unchanged.y - 18.0f) < 0.001f,
           "snapToGrid should no-op for non-positive grid size");
}

class CounterCommand final : public EditorCommand {
public:
    CounterCommand(int* value, int delta, bool mergeable = false)
        : value_(value), delta_(delta), mergeable_(mergeable) {}

    void execute() override { *value_ += delta_; }
    void undo() override { *value_ -= delta_; }
    bool mergeWith(const EditorCommand& other) override {
        if (!mergeable_) return false;
        const auto* rhs = dynamic_cast<const CounterCommand*>(&other);
        if (!rhs || rhs->value_ != value_ || !rhs->mergeable_) return false;
        delta_ += rhs->delta_;
        return true;
    }

private:
    int* value_;
    int delta_;
    bool mergeable_;
};

void testCommandStack() {
    int value = 0;
    CommandStack stack;

    stack.execute(std::make_unique<CounterCommand>(&value, 3));
    expect(value == 3, "Command execute should apply delta");
    expect(stack.canUndo(), "Undo should be available after execute");

    stack.undo();
    expect(value == 0, "Undo should revert delta");
    expect(stack.canRedo(), "Redo should be available after undo");

    stack.redo();
    expect(value == 3, "Redo should reapply delta");

    stack.execute(std::make_unique<CounterCommand>(&value, 2, true));
    stack.execute(std::make_unique<CounterCommand>(&value, 5, true));
    expect(value == 10, "Merged commands should still execute");
    stack.undo();
    expect(value == 3, "Merged undo should revert both merged deltas");
}

void testDuplicateSelectionUndoRedo() {
    auto simulator = std::make_shared<CircuitSimulator>();

    auto* input = static_cast<InputSource*>(simulator->addGate(
        std::make_unique<InputSource>("in", Vector2{0.0f, 0.0f}, Vector2{50.0f, 50.0f}, "IN")));
    auto* notGate = static_cast<NotGate*>(simulator->addGate(
        std::make_unique<NotGate>("n1", Vector2{120.0f, 0.0f}, 100.0f, 60.0f)));

    expect(input && notGate, "Failed to create duplicate-selection test gates");
    expect(simulator->createWire(input->getOutputPin(0), notGate->getInputPin(0)) != nullptr,
           "Failed to create source wire for duplicate-selection test");

    EditorSelection selection;
    selection.addGate(input);
    selection.addGate(notGate);

    CommandStack stack;
    stack.execute(std::make_unique<DuplicateSelectionCommand>(simulator, selection, Vector2{40.0f, 30.0f}));

    expect(simulator->getGates().size() == 4, "Duplicate should add two gates");
    expect(simulator->getWires().size() == 2, "Duplicate should clone internal wire");

    stack.undo();
    expect(simulator->getGates().size() == 2, "Undo duplicate should restore original gate count");
    expect(simulator->getWires().size() == 1, "Undo duplicate should restore original wire count");

    stack.redo();
    expect(simulator->getGates().size() == 4, "Redo duplicate should re-add duplicated gates");
    expect(simulator->getWires().size() == 2, "Redo duplicate should re-add duplicated wire");
}

void testWireDragMaintainsOrthogonalSegments() {
    CircuitSimulator simulator;

    auto* input = static_cast<InputSource*>(simulator.addGate(
        std::make_unique<InputSource>("in", Vector2{0.0f, 0.0f}, Vector2{50.0f, 50.0f}, "IN")));
    auto* notGate = static_cast<NotGate*>(simulator.addGate(
        std::make_unique<NotGate>("n1", Vector2{220.0f, 120.0f}, 100.0f, 60.0f)));

    expect(input && notGate, "Failed to create wire-drag test gates");
    Wire* wire = simulator.createWire(input->getOutputPin(0), notGate->getInputPin(0));
    expect(wire != nullptr, "Failed to create wire-drag test wire");
    expect(wire->getControlPoints().size() >= 3, "Wire should contain draggable internal points");

    const std::vector<Vector2>& initial = wire->getControlPoints();
    const Vector2 dragHandle = initial[1];
    expect(wire->startDraggingPoint(dragHandle, 10.0f), "Expected to start dragging wire elbow");
    wire->updateDraggedPoint({dragHandle.x + 75.0f, dragHandle.y + 42.0f});
    wire->stopDraggingPoint();

    const std::vector<Vector2>& path = wire->getControlPoints();
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const bool horizontal = fabsf(path[i].y - path[i + 1].y) < 0.001f;
        const bool vertical = fabsf(path[i].x - path[i + 1].x) < 0.001f;
        expect(horizontal || vertical, "Wire segment must stay orthogonal after elbow drag");
    }
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Truth tables", testTruthTables},
        {"Depth propagation and order invariance", testDepthPropagationAndOrderInvariance},
        {"Fanout propagation", testFanoutPropagation},
        {"Oscillation detection", testOscillationDetection},
        {"Gate deletion cleanup", testGateDeletionCleansWires},
        {"Interaction helpers", testInteractionHelpers},
        {"Gate geometry pin anchors", testGateGeometryPinAnchorParity},
        {"Gate geometry hit test", testGateGeometryHitTest},
        {"OR/XOR geometry differentiation", testOrXorGeometryDifferentiation},
        {"Curved gate pin boundary contact", testCurvedGatePinBoundaryContact},
        {"snapToGrid helper", testSnapToGridHelper},
        {"Command stack", testCommandStack},
        {"Duplicate selection undo/redo", testDuplicateSelectionUndoRedo},
        {"Wire drag orthogonality", testWireDragMaintainsOrthogonalSegments}
    };

    int passed = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            ++passed;
            std::cout << "[PASS] " << test.first << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[FAIL] " << test.first << ": " << e.what() << "\n";
            return 1;
        }
    }

    std::cout << "All " << passed << " tests passed.\n";
    return 0;
}
