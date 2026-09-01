#pragma once

enum class ItemType
{
    Coin,
    Heal,
    Kunai,
    Haku
};

struct DropData
{
    ItemType type;
    float probability; // 0.0Å`1.0
};