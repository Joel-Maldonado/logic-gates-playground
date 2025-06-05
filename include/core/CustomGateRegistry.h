#ifndef CUSTOMGATEREGISTRY_H
#define CUSTOMGATEREGISTRY_H

#include <string>
#include <vector>
#include <map>
#include <memory> // Though not strictly needed for this header, good practice if unique_ptr might be used later.
#include "core/CustomGateData.h" // Contains the definition structure and nlohmann/json integration

class CustomGateRegistry {
public:
    CustomGateRegistry(const std::string& definitionsPath);

    /**
     * Scans the definitionsPath_ for .json files, parses them into CustomGateData,
     * and stores them in the registry.
     */
    void loadDefinitions();

    /**
     * Retrieves a custom gate definition by its name.
     * @param name The name of the custom gate definition to retrieve.
     * @param outDefinition Reference to a CustomGateData object to store the found definition.
     * @return True if the definition was found, false otherwise.
     */
    bool getDefinition(const std::string& name, CustomGateData& outDefinition) const;

    /**
     * Returns a list of names of all loaded custom gate definitions.
     * @return A vector of strings, where each string is a definition name.
     */
    std::vector<std::string> getAvailableDefinitionNames() const;

    /**
     * Returns a constant reference to the map of all loaded definitions.
     * Useful for iterating through all available custom gates, e.g., for UI display.
     * @return A const reference to the internal registry map.
     */
    const std::map<std::string, CustomGateData>& getAllDefinitions() const;

    /**
     * Saves a custom gate definition to a .json file in the definitionsPath_.
     * The filename will typically be definition.name + ".json".
     * @param definition The CustomGateData object to save.
     * @return True if saving was successful, false otherwise.
     * (Note: Implementation might be basic or a stub for now)
     */
    bool saveDefinition(const CustomGateData& definition);

private:
    std::map<std::string, CustomGateData> registry_;
    std::string definitionsPath_;
};

#endif // CUSTOMGATEREGISTRY_H
