#pragma once

#include "Vector2d.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Hitbox
{
    Vector2d offset;
    float width = 0;
    float height = 0;
};

struct AttackHitbox
{
    std::string name;
    std::vector<Hitbox> hitBox;
    int damage = 0;
};

class HitboxData
{
public:
    bool LoadHitboxes(const char* filePath);
    bool LoadAttackHitboxes(const char* filePath);

    const std::vector<Hitbox>& GetHitboxes() const { return m_hitboxes; }
    const std::vector<AttackHitbox>& GetAttackHitboxes() const { return m_attackHitboxes; }

private:
    std::vector<Hitbox> m_hitboxes;
    std::vector<AttackHitbox> m_attackHitboxes;
    Hitbox LoadHitbox(const nlohmann::json& jsonHitbox);
};
