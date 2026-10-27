#include "MapMenu.h"

#include "Menu.h"
#include "Game.h"

#include <DxLib.h>
#include <cstring>

MapMenu::MapMenu(Menu* owner)
    : MenuPage(owner)
{
}

void MapMenu::Initialize()
{
}

void MapMenu::Update(float deltaTime)
{
    // TODO: Q/E(R stick) で拡大縮小、WASD(L stick) でスクロール
}

void MapMenu::Draw()
{
    // 仮表示: 枠とタイトルだけ
    DrawBox(60, 110, 1220, 660, GetColor(255, 255, 255), FALSE);

    const char* title = "MAP";
    const int w = GetDrawStringWidth(title, static_cast<int>(std::strlen(title)));
    DrawString((1280 - w) / 2, 370, title, GetColor(255, 255, 255));
}

void MapMenu::ClampCursor()
{
}
