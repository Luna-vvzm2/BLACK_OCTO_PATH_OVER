#include "HitboxData.h"
#include <fstream>

using json = nlohmann::json;

bool HitboxData::LoadHitboxes(const char* filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        return false;
    }

    nlohmann::json data;
    file >> data;

    m_hitboxes.clear();

    for (const auto& jsonHitbox : data["hitboxes"])
    {
        m_hitboxes.push_back(LoadHitbox(jsonHitbox));
    }

    return true;
}

bool HitboxData::LoadAttackHitboxes(const char* filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        return false;
    }

    nlohmann::json data;
    file >> data;

    m_attackHitboxes.clear();

    for (const auto& jsonAttack : data["attackHitboxes"])
    {
        AttackHitbox attack;

        attack.name = jsonAttack["name"];
        attack.damage = jsonAttack["damage"];

        for (const auto& jsonHitbox : jsonAttack["hitBox"])
        {
            attack.hitBox.push_back(LoadHitbox(jsonHitbox));
        }

        m_attackHitboxes.push_back(attack);
    }

    return true;
}

Hitbox HitboxData::LoadHitbox(const nlohmann::json& jsonHitbox)
{
    Hitbox hitbox;

    hitbox.offset.x = jsonHitbox["offset"]["x"];
    hitbox.offset.y = jsonHitbox["offset"]["y"];
    hitbox.width = jsonHitbox["width"];
    hitbox.height = jsonHitbox["height"];

    return hitbox;
}