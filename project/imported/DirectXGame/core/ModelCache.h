#pragma once

#include <KamataEngine.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace DirectXGame {

class ModelCache {
public:
    static std::shared_ptr<KamataEngine::Model> Get(const std::string& modelName, bool smoothing = false) {
        auto& cache = GetCache();
        const std::string key = modelName + (smoothing ? "#smooth" : "#flat");

        auto it = cache.find(key);
        if (it != cache.end()) {
            if (auto model = it->second.lock()) {
                return model;
            }
        }

        std::shared_ptr<KamataEngine::Model> model(KamataEngine::Model::CreateFromOBJ(modelName, smoothing));
        cache[key] = model;
        return model;
    }

private:
    static std::unordered_map<std::string, std::weak_ptr<KamataEngine::Model>>& GetCache() {
        static std::unordered_map<std::string, std::weak_ptr<KamataEngine::Model>> cache;
        return cache;
    }
};

} // namespace DirectXGame
