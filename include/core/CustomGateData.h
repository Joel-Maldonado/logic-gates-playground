#ifndef CUSTOMGATEDATA_H
#define CUSTOMGATEDATA_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp> // Added for JSON serialization

struct CustomGateData {
    std::string name;
    std::string category; // For UI organization, e.g., "My Gates", "Logic", etc.
    int numExternalInputPins;
    int numExternalOutputPins;

    struct InternalGateDesc {
        std::string type; /* e.g., "AndGate", "OrGate", "CustomGate:MyPreviousCustom" */
        std::string id;   /* Unique within the custom gate */
        float posX, posY; /* Relative position within the custom gate editor view */
    };

    struct InternalWireDesc {
        std::string fromGateId;
        int fromPinIndex;
        std::string toGateId;
        int toPinIndex;
    };

    struct PinMappingDesc {
        int externalPinIndex;
        std::string internalGateId;
        int internalPinIndex;
        bool isOutputMapping; /* true if mapping an internal output to an external output, false for external input to internal input */
    };

    std::vector<InternalGateDesc> internalGates;
    std::vector<InternalWireDesc> internalWires;
    std::vector<PinMappingDesc> pinMappings;
};

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE for nested structures
// These should be in the same namespace as the structs they describe.
// If the structs are in a namespace, these macros should also be in that namespace.
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CustomGateData::InternalGateDesc, type, id, posX, posY)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CustomGateData::InternalWireDesc, fromGateId, fromPinIndex, toGateId, toPinIndex)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CustomGateData::PinMappingDesc, externalPinIndex, internalGateId, internalPinIndex, isOutputMapping)

// to_json and from_json functions for CustomGateData
// These can be inline in the header or in a .cpp file. For simplicity here, they are inline.
inline void to_json(nlohmann::json& j, const CustomGateData& cgd) {
    j = nlohmann::json{
        {"name", cgd.name},
        {"category", cgd.category},
        {"numExternalInputPins", cgd.numExternalInputPins},
        {"numExternalOutputPins", cgd.numExternalOutputPins},
        {"internalGates", cgd.internalGates},
        {"internalWires", cgd.internalWires},
        {"pinMappings", cgd.pinMappings}
    };
}

inline void from_json(const nlohmann::json& j, CustomGateData& cgd) {
    j.at("name").get_to(cgd.name);
    j.at("category").get_to(cgd.category);
    j.at("numExternalInputPins").get_to(cgd.numExternalInputPins);
    j.at("numExternalOutputPins").get_to(cgd.numExternalOutputPins);
    j.at("internalGates").get_to(cgd.internalGates);
    j.at("internalWires").get_to(cgd.internalWires);
    j.at("pinMappings").get_to(cgd.pinMappings);
}

#endif // CUSTOMGATEDATA_H
