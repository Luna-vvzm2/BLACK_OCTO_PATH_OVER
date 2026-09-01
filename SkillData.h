#pragma once

#include <string>

struct SkillData
{
    std::string name;
    std::string description;

    int iconHandle = -1;

    bool unlocked = true;
};