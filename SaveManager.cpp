#include "SaveManager.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <vector>

using json = nlohmann::json;

bool SaveManager::Save(const SaveData& data)
{
    json saveData;

    saveData["currentStage"] = data.currentStage;
    saveData["stageClear"] = data.stageClear;
    saveData["playTime"] = data.playTime;
    saveData["kunaiCount"] = data.kunaiCount;

    // キー設定を保存
    saveData["keyBindings"] = data.keyBindings;

    // 操作方式を保存
    saveData["inputModes"] = data.inputModes;

    std::ofstream file("save/save.json");

    if (!file.is_open())
    {
        return false;
    }

    file << saveData.dump(4);

    return true;
}

bool SaveManager::Load(SaveData& data)
{
    std::ifstream file("save/save.json");

    if (!file.is_open())
    {
        return false;
    }

    json saveData;

    file >> saveData;

    data.currentStage = saveData.value("currentStage", 0);
    data.playTime = saveData.value("playTime", 0.0f);
    data.kunaiCount = saveData.value("kunaiCount", 10);

    if (saveData.contains("stageClear") &&
        saveData["stageClear"].is_array())
    {
        for (size_t i = 0; i < data.stageClear.size(); ++i)
        {
            if (i < saveData["stageClear"].size())
            {
                data.stageClear[i] =
                    saveData["stageClear"][i].get<bool>();
            }
        }
    }

    // キー割り当てを読み込む
    if (saveData.contains("keyBindings") &&
        saveData["keyBindings"].is_array())
    {
        const auto& savedBindings = saveData["keyBindings"];

        const std::size_t count =
            (savedBindings.size() < data.keyBindings.size())
            ? savedBindings.size()
            : data.keyBindings.size();

        for (std::size_t i = 0; i < count; ++i)
        {
            if (!savedBindings[i].is_array())
                continue;

            std::vector<int> keys;

            for (const auto& value : savedBindings[i])
            {
                if (value.is_number_integer())
                {
                    keys.push_back(value.get<int>());
                }
            }

            // 空の割り当ては読み込まない
            if (!keys.empty())
            {
                data.keyBindings[i] = std::move(keys);
            }
        }
    }

    // 操作方式を読み込む
    if (saveData.contains("inputModes") &&
        saveData["inputModes"].is_array())
    {
        const auto& savedModes = saveData["inputModes"];

        const std::size_t count =
            (savedModes.size() < data.inputModes.size())
            ? savedModes.size()
            : data.inputModes.size();

        for (std::size_t i = 0; i < count; ++i)
        {
            if (!savedModes[i].is_number_integer())
                continue;

            const int mode = savedModes[i].get<int>();

            // 0 = 長押し、1 = トグル
            if (mode == 0 || mode == 1)
            {
                data.inputModes[i] = mode;
            }
        }
    }

    return true;
}