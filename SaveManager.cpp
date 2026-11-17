#include "SaveManager.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
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
    saveData["newsOwned"] = data.newsOwned;

    // キー設定を保存
    saveData["keyBindings"] = data.keyBindings;

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

    if (saveData.contains("newsOwned") &&
        saveData["newsOwned"].is_array())
    {
        for (size_t i = 0; i < data.newsOwned.size(); ++i)
        {
            if (i < saveData["newsOwned"].size())
            {
                data.newsOwned[i] =
                    saveData["newsOwned"][i].get<bool>();
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

            // Recover defaults missing from saves created by the old binding loader.
            const Action action = static_cast<Action>(i);
            if (keys.empty())
            {
                if (action == Action::OCTO) keys = { Key::Q };
                else if (action == Action::TIGER) keys = { Key::E };
                else if (action == Action::SHADOW_HEAL) keys = { Key::Z };
            }

            data.keyBindings[i] = std::move(keys);
            data.hasKeyBinding[i] = true;
        }
    }

    return true;
}