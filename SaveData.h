#pragma once

#include "Input.h"

#include <array>
#include <vector>
#include <cstddef>

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

    // 操作数
    static constexpr std::size_t ActionCount =
        static_cast<std::size_t>(Action::ACTION_COUNT);

    // アクションごとのキー割り当て
    // 1つの操作に複数のキーが設定される場合にも対応
    std::array<std::vector<int>, ActionCount> keyBindings{};

    // 操作方式
    // 0 = 長押し
    // 1 = トグル
    std::array<int, ActionCount> inputModes{};
};