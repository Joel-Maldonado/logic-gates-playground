#include "core/CustomGateRegistry.h"
#include "nlohmann/json.hpp" // For parsing JSON
#include <fstream>           // For std::ifstream
#include <filesystem>        // For directory iteration (C++17)
#include <iostream>          // For error logging (can be replaced with a proper logger)
#include <string>            // For std::string operations in error messages

// Namespace alias for convenience
namespace fs = std::filesystem;

CustomGateRegistry::CustomGateRegistry(const std::string& definitionsPath)
    : definitionsPath_(definitionsPath) {
    if (!fs::exists(definitionsPath_)) {
        std::cerr << "Info (CustomGateRegistry): Custom gate definitions path '" << definitionsPath_
                  << "' does not exist. It will be created if definitions are saved." << std::endl;
    } else if (!fs::is_directory(definitionsPath_)) {
        std::cerr << "Error (CustomGateRegistry): Custom gate definitions path '" << definitionsPath_
                  << "' exists but is not a directory. Registry will not load/save definitions." << std::endl;
        definitionsPath_ = ""; // Invalidate path to prevent usage
    }
}

void CustomGateRegistry::loadDefinitions() {
    registry_.clear();

    if (definitionsPath_.empty() || !fs::exists(definitionsPath_)) {
        std::cerr << "Info (CustomGateRegistry::loadDefinitions): Definitions path is invalid or does not exist: '"
                  << (definitionsPath_.empty() ? "[not set]" : definitionsPath_) << "'. No definitions loaded." << std::endl;
        return;
    }
    if (!fs::is_directory(definitionsPath_)) { // Should be caught by constructor, but double check
         std::cerr << "Error (CustomGateRegistry::loadDefinitions): Definitions path '" << definitionsPath_
                   << "' is not a directory. No definitions loaded." << std::endl;
        return;
    }


    std::cout << "Loading custom gate definitions from: " << definitionsPath_ << std::endl;
    for (const auto& entry : fs::directory_iterator(definitionsPath_)) {
        const fs::path& path = entry.path();
        if (entry.is_regular_file() && path.extension() == ".json") {
            std::ifstream fileStream(path);
            if (!fileStream.is_open()) {
                std::cerr << "Error (CustomGateRegistry): Could not open custom gate definition file: " << path.string() << std::endl;
                continue;
            }

            CustomGateData definition;
            try {
                nlohmann::json jsonData;
                fileStream >> jsonData;

                // Use .at() for required fields, which throws if missing.
                // CustomGateData::from_json already uses .at(), so errors are caught below.
                definition = jsonData.get<CustomGateData>(); // This uses the from_json in CustomGateData.h

                // Basic validation after successful parsing
                if (definition.name.empty()) {
                    std::cerr << "Error (CustomGateRegistry): Definition from file " << path.string() << " is missing 'name'. Skipping." << std::endl;
                    continue;
                }
                if (definition.numExternalInputPins < 0) {
                    std::cerr << "Error (CustomGateRegistry): Definition '" << definition.name << "' from " << path.string()
                              << " has negative numExternalInputPins (" << definition.numExternalInputPins << "). Skipping." << std::endl;
                    continue;
                }
                if (definition.numExternalOutputPins < 0) {
                    std::cerr << "Error (CustomGateRegistry): Definition '" << definition.name << "' from " << path.string()
                              << " has negative numExternalOutputPins (" << definition.numExternalOutputPins << "). Skipping." << std::endl;
                    continue;
                }
                // Check for presence of main structural vectors, even if empty. nlohmann's from_json for vectors handles missing keys if the default constructor for the vector is acceptable.
                // If these keys *must* exist in the JSON, CustomGateData::from_json should enforce it.
                // The current CustomGateData::from_json uses j.at(...).get_to(), so it does enforce presence.

                if (registry_.count(definition.name)) {
                     std::cerr << "Warning (CustomGateRegistry): Duplicate custom gate definition name '" << definition.name
                               << "' found in file " << path.string() << ". Previous definition will be overwritten." << std::endl;
                }
                registry_[definition.name] = definition;
                std::cout << "Successfully loaded custom gate definition: '" << definition.name << "' from " << path.string() << std::endl;

            } catch (const nlohmann::json::parse_error& e) {
                std::cerr << "Error (CustomGateRegistry): JSON parsing error in file " << path.string() << " at byte " << e.byte << " - " << e.what() << std::endl;
            } catch (const nlohmann::json::type_error& e) { // Catches wrong types for fields
                std::cerr << "Error (CustomGateRegistry): JSON type error in file " << path.string() << " - " << e.what() << std::endl;
            } catch (const nlohmann::json::out_of_range& e) { // Catches missing fields accessed by .at()
                std::cerr << "Error (CustomGateRegistry): JSON missing field error in file " << path.string() << " - " << e.what() << std::endl;
            } catch (const std::exception& e) { // Catch-all for other C++ exceptions
                std::cerr << "Error (CustomGateRegistry): Failed to load custom gate definition from " << path.string() << " - " << e.what() << std::endl;
            }
        }
    }
}

bool CustomGateRegistry::getDefinition(const std::string& name, CustomGateData& outDefinition) const {
    auto it = registry_.find(name);
    if (it != registry_.end()) {
        outDefinition = it->second; // Returns a copy
        return true;
    }
    std::cerr << "Warning (CustomGateRegistry::getDefinition): No definition found for name '" << name << "'." << std::endl;
    return false;
}

std::vector<std::string> CustomGateRegistry::getAvailableDefinitionNames() const {
    std::vector<std::string> names;
    names.reserve(registry_.size());
    for (const auto& pair : registry_) {
        names.push_back(pair.first);
    }
    return names;
}

const std::map<std::string, CustomGateData>& CustomGateRegistry::getAllDefinitions() const {
    return registry_;
}

bool CustomGateRegistry::saveDefinition(const CustomGateData& definition) {
    if (definitionsPath_.empty()) {
        std::cerr << "Error (CustomGateRegistry::saveDefinition): Definitions path is not set or invalid. Cannot save." << std::endl;
        return false;
    }
    if (definition.name.empty()) {
        std::cerr << "Error (CustomGateRegistry::saveDefinition): Cannot save custom gate definition without a 'name'." << std::endl;
        return false;
    }
     if (definition.numExternalInputPins < 0 || definition.numExternalOutputPins < 0) {
        std::cerr << "Error (CustomGateRegistry::saveDefinition): Definition '" << definition.name
                  << "' has negative pin counts. Cannot save." << std::endl;
        return false;
    }


    if (!fs::exists(definitionsPath_)) {
        std::cout << "Info (CustomGateRegistry::saveDefinition): Definitions directory '" << definitionsPath_
                  << "' does not exist. Attempting to create it." << std::endl;
        try {
            if (!fs::create_directories(definitionsPath_)) {
                 std::cerr << "Error (CustomGateRegistry::saveDefinition): Failed to create definitions directory '"
                           << definitionsPath_ << "' (create_directories returned false)." << std::endl;
                return false;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error (CustomGateRegistry::saveDefinition): Could not create definitions directory '"
                      << definitionsPath_ << "' - " << e.what() << std::endl;
            return false;
        }
    }

    fs::path filePath = fs::path(definitionsPath_) / (definition.name + ".json");

    try {
        nlohmann::json jsonData = definition; // This uses the to_json from CustomGateData.h

        std::ofstream fileStream(filePath);
        if (!fileStream.is_open()) {
            std::cerr << "Error (CustomGateRegistry::saveDefinition): Could not open file for saving: '" << filePath.string() << "'." << std::endl;
            return false;
        }

        fileStream << jsonData.dump(4); // Dump with an indent of 4 for readability
        fileStream.close(); // Explicitly close to check for errors

        if (fileStream.fail()) {
            std::cerr << "Error (CustomGateRegistry::saveDefinition): Failed to write all data to file: '" << filePath.string() << "'." << std::endl;
            // Attempt to remove partially written file
            fs::remove(filePath);
            return false;
        }

        std::cout << "Successfully saved custom gate definition: '" << definition.name << "' to " << filePath.string() << std::endl;

        // Update the in-memory registry immediately
        registry_[definition.name] = definition;
        return true;

    } catch (const nlohmann::json::exception& e) { // Covers errors from to_json as well
        std::cerr << "Error (CustomGateRegistry::saveDefinition): JSON serialization error for definition '" << definition.name << "' - " << e.what() << std::endl;
    } catch (const fs::filesystem_error& e) { // Catch filesystem errors specifically if they occur outside directory creation
        std::cerr << "Error (CustomGateRegistry::saveDefinition): Filesystem error for definition '" << definition.name << "' at path '" << filePath.string() << "' - " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error (CustomGateRegistry::saveDefinition): Failed to save custom gate definition '" << definition.name << "' to '" << filePath.string() << "' - " << e.what() << std::endl;
    }
    return false;
}
