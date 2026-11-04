#pragma once

#include <array>

struct SaveData
{
    // 現在プレイしているステージ
    int currentStage = 0;

    // 各ステージのクリア状況
    // 0 = 未クリア
    // 1 = クリア
    std::array<bool, 3> stageClear{ false, false, false };

    // 総プレイ時間（秒）
    float playTime = 0.0f;

    // 所持手裏剣数
    int kunaiCount = 10;

    // 入手済みの新聞(新聞ID 1~6 → 添字 0~5)
    std::array<bool, 6> newsOwned{ true, false, false, false, false, false };
};
