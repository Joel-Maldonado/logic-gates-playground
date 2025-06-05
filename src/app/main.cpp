#include "app/Application.h"
#include "core/CustomGate.h"
#include "core/CustomGateData.h"
#include "core/DerivedGates.h" // For AndGate, OrGate, NotGate
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "simulation/CircuitSimulator.h"
#include "core/Wire.h" // For connecting test circuit
#include <iostream>
#include <vector>
#include <memory> // For std::make_unique

// Test function for CustomGate with XNOR logic
void runCustomGateXnorTest() {
    std::cout << "--- Starting CustomGate XNOR Test ---" << std::endl;

    CustomGateData xnorData;
    xnorData.name = "TestXNOR";
    xnorData.category = "TestCategory";
    xnorData.numExternalInputPins = 2;
    xnorData.numExternalOutputPins = 1;

    // Internal Gate Descriptions
    // These are sources *inside* the custom gate, driven by its external inputs via pin mapping
    xnorData.internalGates.push_back({"InputSource", "cg_in_A_source", 50, 50}); // Input for A
    xnorData.internalGates.push_back({"InputSource", "cg_in_B_source", 50, 150});// Input for B

    xnorData.internalGates.push_back({"NotGate", "notA", 150, 50});
    xnorData.internalGates.push_back({"NotGate", "notB", 150, 150});

    xnorData.internalGates.push_back({"AndGate", "and_A_B", 250, 75});      // A AND B
    xnorData.internalGates.push_back({"AndGate", "and_notA_notB", 250, 175}); // (NOT A) AND (NOT B)

    xnorData.internalGates.push_back({"OrGate", "or_final", 350, 125}); // (A AND B) OR ((NOT A) AND (NOT B))

    // This is a sink *inside* the custom gate, its state will drive the custom gate's external output via pin mapping
    xnorData.internalGates.push_back({"OutputSink", "cg_out_Q_sink", 450, 125}); // Output Q

    // Internal Wire Descriptions
    // Connect cg_in_A_source to notA and and_A_B
    xnorData.internalWires.push_back({"cg_in_A_source", 0, "notA", 0});
    xnorData.internalWires.push_back({"cg_in_A_source", 0, "and_A_B", 0});

    // Connect cg_in_B_source to notB and and_A_B
    xnorData.internalWires.push_back({"cg_in_B_source", 0, "notB", 0});
    xnorData.internalWires.push_back({"cg_in_B_source", 0, "and_A_B", 1});

    // Connect notA to and_notA_notB
    xnorData.internalWires.push_back({"notA", 0, "and_notA_notB", 0});
    // Connect notB to and_notA_notB
    xnorData.internalWires.push_back({"notB", 0, "and_notA_notB", 1});

    // Connect and_A_B to or_final
    xnorData.internalWires.push_back({"and_A_B", 0, "or_final", 0});
    // Connect and_notA_notB to or_final
    xnorData.internalWires.push_back({"and_notA_notB", 0, "or_final", 1});

    // Connect or_final to cg_out_Q_sink
    xnorData.internalWires.push_back({"or_final", 0, "cg_out_Q_sink", 0});

    // Pin Mapping Descriptions
    // Map CustomGate external input 0 to internal InputSource "cg_in_A_source" (output pin 0 of InputSource)
    xnorData.pinMappings.push_back({0, "cg_in_A_source", 0, false}); // externalPinIndex, internalGateId, internalPinIndex, isOutputMapping=false
    // Map CustomGate external input 1 to internal InputSource "cg_in_B_source" (output pin 0 of InputSource)
    xnorData.pinMappings.push_back({1, "cg_in_B_source", 0, false}); // isOutputMapping=false means external input maps to internal gate's input pin (which is how InputSource is modeled internally)

    // Actually, for InputSource, we are mapping the CustomGate's input to *drive* the InputSource.
    // The CustomGate's setupInternalCircuit should handle InputSource specially.
    // The current CustomGate::evaluate() propagates CustomGate input pin states to mapped *internal input pins*.
    // InputSource only has output pins in LogicGate model.
    // This means the pin mapping for InputSource needs to be to its "state" effectively, not a pin.
    // Let's re-evaluate CustomGate::evaluate and setup for InputSource mapping.
    // For now, we assume that mapping an external input to an InputSource's "pin 0" (its output) will effectively set its state.
    // This might require a small adjustment in CustomGate::evaluate if InputSource is the target of a mapping.
    // The current CustomGate::evaluate logic: externalToInternalInputPinMap_[i]->setState(inputPins_[i].getState());
    // This means cg_in_A_source and cg_in_B_source should be simple pass-through gates or buffers if we map to their input pins.
    // Or, if they are InputSource, the mapping externalPinIndex -> internalGateId (InputSource), internalPinIndex (0 for output)
    // must be interpreted by CustomGate::evaluate to call internalGate->setState() if internalGate is an InputSource.
    // The current CustomGate::setupInternalCircuit for InputSource in pinMappings:
    // externalToInternalInputPinMap_[mapping.externalPinIndex] = internalGate->getInputPin(mapping.internalPinIndex);
    // InputSource has NO input pins by default in LogicGate. It only has one output pin.
    // This is a conceptual mismatch.
    //
    // **Correction Plan for Pin Mapping to Internal InputSource/OutputSink:**
    // 1. Internal "sources" for external inputs should NOT be InputSource gates. They should be regular gates like a small buffer or even just directly wire to the inputs of the first logic gates (e.g. notA, and_A_B).
    // 2. Internal "sinks" for external outputs should NOT be OutputSink gates. The output of the last internal logic gate (e.g. or_final) should be mapped directly.
    //
    // Let's redefine internal components and mappings:
    // Remove cg_in_A_source, cg_in_B_source, cg_out_Q_sink from internalGates and wires.
    // The pin mappings will now point directly to the logic gates' pins.

    // Revised CustomGateData:
    xnorData.internalGates.clear();
    xnorData.internalWires.clear();
    xnorData.pinMappings.clear();

    // Internal Gates (Actual Logic for XNOR)
    xnorData.internalGates.push_back({"NotGate", "notA", 50, 50});
    xnorData.internalGates.push_back({"NotGate", "notB", 50, 150});
    xnorData.internalGates.push_back({"AndGate", "and_A_B", 150, 75});      // A AND B
    xnorData.internalGates.push_back({"AndGate", "and_notA_notB", 150, 175}); // (NOT A) AND (NOT B)
    xnorData.internalGates.push_back({"OrGate", "or_final", 250, 125}); // (A AND B) OR ((NOT A) AND (NOT B))

    // Internal Wires (Wiring the XNOR logic)
    // A (from external) -> notA input 0, and_A_B input 0
    // B (from external) -> notB input 0, and_A_B input 1
    // These connections are established by pin mappings.

    xnorData.internalWires.push_back({"notA", 0, "and_notA_notB", 0});
    xnorData.internalWires.push_back({"notB", 0, "and_notA_notB", 1});
    xnorData.internalWires.push_back({"and_A_B", 0, "or_final", 0});
    xnorData.internalWires.push_back({"and_notA_notB", 0, "or_final", 1});

    // Pin Mappings (Connecting external pins to internal logic pins)
    // External Input 0 (A) maps to:
    xnorData.pinMappings.push_back({0, "notA", 0, false});    // CustomGate Input 0 to notA's input pin 0
    xnorData.pinMappings.push_back({0, "and_A_B", 0, false}); // CustomGate Input 0 to and_A_B's input pin 0

    // External Input 1 (B) maps to:
    xnorData.pinMappings.push_back({1, "notB", 0, false});    // CustomGate Input 1 to notB's input pin 0
    xnorData.pinMappings.push_back({1, "and_A_B", 1, false}); // CustomGate Input 1 to and_A_B's input pin 1

    // External Output 0 (Q) maps from:
    xnorData.pinMappings.push_back({0, "or_final", 0, true}); // CustomGate Output 0 from or_final's output pin 0

    // --- Simulator Setup ---
    CircuitSimulator simulator(true); // true for enabling event logging from simulator

    // Create the CustomGate
    // Note: CustomGate constructor uses default width/height. For this test, it's fine.
    auto customXnorGate = std::make_unique<CustomGate>("cg_xnor", Vector2{200, 200}, xnorData);

    // External test rig: Input sources and Output sink for the simulator
    auto masterInputA = std::make_unique<InputSource>("masterInA", Vector2{50, 100}, Vector2{20,20}, "A");
    auto masterInputB = std::make_unique<InputSource>("masterInB", Vector2{50, 300}, Vector2{20,20}, "B");
    auto masterOutputQ = std::make_unique<OutputSink>("masterOutQ", Vector2{350, 200}, 10.0f, "Q");

    InputSource* pMasterInputA = masterInputA.get();
    InputSource* pMasterInputB = masterInputB.get();
    OutputSink* pMasterOutputQ = masterOutputQ.get();
    CustomGate* pCustomXnorGate = customXnorGate.get();

    simulator.addGate(std::move(masterInputA));
    simulator.addGate(std::move(masterInputB));
    simulator.addGate(std::move(customXnorGate)); // Add custom gate
    simulator.addGate(std::move(masterOutputQ));

    // Wire up the external test rig to the CustomGate
    simulator.addWire(std::make_unique<Wire>("w_masterA_cgXnor", pMasterInputA->getOutputPin(0), pCustomXnorGate->getInputPin(0)));
    simulator.addWire(std::make_unique<Wire>("w_masterB_cgXnor", pMasterInputB->getOutputPin(0), pCustomXnorGate->getInputPin(1)));
    simulator.addWire(std::make_unique<Wire>("w_cgXnor_masterQ", pCustomXnorGate->getOutputPin(0), pMasterOutputQ->getInputPin(0)));

    std::cout << "Simulator setup complete. Starting truth table tests..." << std::endl;

    // Test Cases for XNOR
    bool testInputs[4][2] = {
        {false, false}, // Expected: true
        {false, true},  // Expected: false
        {true, false},  // Expected: false
        {true, true}    // Expected: true
    };
    bool expectedOutputs[4] = {true, false, false, true};
    bool allTestsPassed = true;

    for (int i = 0; i < 4; ++i) {
        bool inputA = testInputs[i][0];
        bool inputB = testInputs[i][1];
        bool expected = expectedOutputs[i];

        pMasterInputA->setState(inputA);
        pMasterInputB->setState(inputB);

        // simulator.evaluateAll(); // Initial propagation from new input states
        // simulator.evaluateAll(); // Allow stabilization
        // simulator.evaluateAll();
        // The CircuitSimulator::update() should handle iterative evaluation if needed.
        // For this test, let's call it a few times to be sure.
        // The CustomGate's internal simulator also calls evaluateAll.
        simulator.update();
        simulator.update();
        simulator.update();


        bool actualOutput = pMasterOutputQ->isActive(); // OutputSink::isActive() or ::getState()

        std::cout << "Test Case " << (i + 1) << ": A=" << inputA << ", B=" << inputB
                  << " | Expected Q=" << expected << ", Actual Q=" << actualOutput
                  << " | " << (actualOutput == expected ? "PASS" : "FAIL") << std::endl;

        if (actualOutput != expected) {
            allTestsPassed = false;
        }
    }

    if (allTestsPassed) {
        std::cout << "--- CustomGate XNOR Test: ALL TESTS PASSED ---" << std::endl;
    } else {
        std::cout << "--- CustomGate XNOR Test: SOME TESTS FAILED ---" << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    // Run the test
    runCustomGateXnorTest();

    // Original application code (can be commented out if test takes too long or interferes)
    Application app;
    app.initialize();
    app.run();
    return 0;
}
