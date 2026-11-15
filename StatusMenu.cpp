#include "StatusMenu.h"

#include "Menu.h"
#include "Game.h"
#include "PlayerEntity.h"
#include "HPComponent.h"

#include <DxLib.h>
#include <cstring>

namespace
{
    constexpr int HEAL_ITEM_MAX = 4;   // 仕様書: 回復アイテム最大4個

    void DrawGauge(int x, int y, int w, int h, int cur, int max, unsigned int color)
    {
        DrawBox(x, y, x + w, y + h, GetColor(50, 50, 50), TRUE);

        if (max > 0)
        {
            if (cur < 0) cur = 0;
            if (cur > max) cur = max;

            const int fill = w * cur / max;
            if (fill > 0)
            {
                DrawBox(x, y, x + fill, y + h, color, TRUE);
            }
        }

        DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), FALSE);
    }
}

StatusMenu::StatusMenu(Menu* owner)
    : MenuPage(owner)
{
}

void StatusMenu::Initialize()
{
}

void StatusMenu::Update(float deltaTime)
{
    // 閲覧のみ
}

void StatusMenu::Draw()
{
    PlayerEntity* player = m_owner->GetPlayer();
    if (player == nullptr)
    {
        return;
    }

    const unsigned int white = GetColor(255, 255, 255);
    const int x = 80;

    //----------------------------------
    // HP
    //----------------------------------
    int hp = 0;
    int hpMax = 0;
    if (player->GetHP() != nullptr)
    {
        hp = player->GetHP()->GetHP();
        hpMax = player->GetHP()->GetMaxHP();
    }

    DrawString(x, 110, "HP", white);
    DrawGauge(x, 135, 400, 22, hp, hpMax, GetColor(60, 200, 60));
    DrawFormatString(x + 420, 139, white, "%d / %d", hp, hpMax);

    //----------------------------------
    // 影ゲージ
    //----------------------------------
    const int shadow = player->GetShadowGauge();
    const int shadowMax = player->GetShadowGaugeMax();

    DrawString(x, 190, "影ゲージ", white);
    DrawGauge(x, 215, 400, 22, shadow, shadowMax, GetColor(130, 70, 200));
    DrawFormatString(x + 420, 219, white, "%d / %d", shadow, shadowMax);

    //----------------------------------
    // 所持忍術 (蛸 / 虎 / 蛙 / 鯱)
    //----------------------------------
    static const char* names[4] = { "蛸", "虎", "蛙", "鯱" };

    DrawString(x, 280, "所持忍術", white);

    for (int i = 0; i < 4; i++)
    {
        const int bx = x + i * 110;
        const int by = 310;
        const int size = 90;

        const bool owned = player->GetOwnJutsu(i);
        const unsigned int color = owned ? white : GetColor(90, 90, 90);

        DrawBox(bx, by, bx + size, by + size, color, FALSE);

        const int w = GetDrawStringWidth(
            names[i],
            static_cast<int>(std::strlen(names[i]))
        );
        DrawString(bx + (size - w) / 2, by + 37, names[i], color);
    }

    //----------------------------------
    // 所持アイテム
    //----------------------------------
    DrawString(x, 440, "所持アイテム", white);
    DrawFormatString(x, 470, white, "クナイ          %d", player->GetKunai());
    DrawFormatString(x, 500, white, "回復アイテム  %d / %d", player->GetHealItem(), HEAL_ITEM_MAX);
}

void StatusMenu::ClampCursor()
{
}
