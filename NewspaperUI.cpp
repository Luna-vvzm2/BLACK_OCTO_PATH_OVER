#include "NewspaperUI.h"

void NewspaperUI::Draw() const {
    // 背景（右 2/3：紙色）
    DrawBox(0, 0, kScreenW, kScreenH, GetColor(230, 220, 195), TRUE);

    // 左 1/3：リスト枠
    DrawBox(0, 0, kLeftW, kScreenH, GetColor(60, 50, 80), TRUE);
    DrawLine(kLeftW, 0, kLeftW, kScreenH, GetColor(0, 0, 0), 4);

    // 左：リスト項目（仮）
    for (int i = 0; i < 3; i++) {
        int y = 80 + i * 100;
        DrawBox(20, y, kLeftW - 20, y + 90, GetColor(190, 60, 200), i == 0);
    }

    // 右：見出し・日付の位置（仮の枠）
    DrawBox(680, 60, 1340, 130, GetColor(120, 110, 90), FALSE);  // 見出し
    DrawBox(680, 150, 1000, 190, GetColor(120, 110, 90), FALSE);  // 日付

    // 右：画像枠
    DrawBox(1380, 60, 1860, 360, GetColor(255, 255, 255), TRUE);
    DrawBox(1380, 60, 1860, 360, GetColor(0, 0, 0), FALSE);

    // 右：本文エリア
    DrawBox(680, 400, 1860, 1000, GetColor(200, 190, 165), FALSE);
}