#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace DirectXGame::UILayoutIO {

struct Entry {
    std::string key;
    std::vector<float> values;
};

using LayoutMap = std::unordered_map<std::string, std::vector<float>>;

LayoutMap Load(const std::string& filePath);
bool Save(const std::string& filePath, const std::vector<Entry>& entries);

} // namespace DirectXGame::UILayoutIO
