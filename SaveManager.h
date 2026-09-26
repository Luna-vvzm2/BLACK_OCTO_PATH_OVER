#pragma once

#include "SaveData.h"

class SaveManager
{
public:
    // セーブデータをJSONに保存
    static bool Save(const SaveData& data);

    // JSONからセーブデータを読み込み
    static bool Load(SaveData& data);
};
