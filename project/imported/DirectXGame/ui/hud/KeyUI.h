#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

class KeyUI {
public:
    void Initialize();
    void Update(KamataEngine::Input* input);
    void Draw();

private:
    std::unique_ptr<KamataEngine::Sprite> keyW_;
    std::unique_ptr<KamataEngine::Sprite> keyA_;
    std::unique_ptr<KamataEngine::Sprite> keyS_;
    std::unique_ptr<KamataEngine::Sprite> keyD_;

    std::unique_ptr<KamataEngine::Sprite> keyESC_;

    void SetKeyColor(KamataEngine::Sprite* key, bool pressed);
};

} // namespace DirectXGame
