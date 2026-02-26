#include "core/DerivedGates.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "simulation/CircuitSimulator.h"
#include "ui/InteractionHelpers.h"

#include <algorithm>
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

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Truth tables", testTruthTables},
        {"Depth propagation and order invariance", testDepthPropagationAndOrderInvariance},
        {"Fanout propagation", testFanoutPropagation},
        {"Oscillation detection", testOscillationDetection},
        {"Gate deletion cleanup", testGateDeletionCleansWires},
        {"Interaction helpers", testInteractionHelpers}
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
