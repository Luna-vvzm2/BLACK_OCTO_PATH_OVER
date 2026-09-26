#include "SaveManager.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

bool SaveManager::Save(const SaveData& data)
{
    json saveData;

    saveData["currentStage"] = data.currentStage;
    saveData["stageClear"] = data.stageClear;
    saveData["playTime"] = data.playTime;
    saveData["kunaiCount"] = data.kunaiCount;

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

    return true;
}