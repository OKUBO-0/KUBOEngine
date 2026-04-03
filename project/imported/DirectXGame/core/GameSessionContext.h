#pragma once

#include <cstdint>

namespace DirectXGame {

struct ResultData {
    int32_t totalExp = 0;
    int32_t finalLevel = 1;
    int32_t totalKillCount = 0;
};

struct GameSessionContext {
    ResultData resultData{};
};

} // namespace DirectXGame
