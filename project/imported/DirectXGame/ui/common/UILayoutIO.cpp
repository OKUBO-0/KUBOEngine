#include "UILayoutIO.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace DirectXGame::UILayoutIO {

LayoutMap Load(const std::string& filePath) {
    LayoutMap result;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string key;
        std::getline(ss, key, ',');
        if (key.empty()) {
            continue;
        }

        std::vector<float> values;
        std::string valueText;
        while (std::getline(ss, valueText, ',')) {
            if (valueText.empty()) {
                continue;
            }
            values.push_back(std::stof(valueText));
        }

        result[key] = std::move(values);
    }

    return result;
}

bool Save(const std::string& filePath, const std::vector<Entry>& entries) {
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

    LayoutMap merged = Load(filePath);
    for (const auto& entry : entries) {
        merged[entry.key] = entry.values;
    }

    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& [key, values] : merged) {
        file << key;
        for (float value : values) {
            file << ',' << value;
        }
        file << '\n';
    }

    return true;
}

} // namespace DirectXGame::UILayoutIO
