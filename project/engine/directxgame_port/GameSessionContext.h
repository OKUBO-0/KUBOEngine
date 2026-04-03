#pragma once

#include <cstdint>

namespace DirectXGamePort {

struct ResultData {
    int32_t totalExp = 0;
    int32_t finalLevel = 1;
    int32_t totalKillCount = 0;
};

struct GameSessionContext {
    ResultData resultData{};
};

} // namespace DirectXGamePort
