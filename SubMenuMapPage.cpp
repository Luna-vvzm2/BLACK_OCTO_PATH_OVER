#include "SubMenuMapPage.h"
#include "SubMenu.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Scene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "DxLib.h"
#include <algorithm>
#include <string>

using std::min;
using std::max;

namespace
{
    // 枠付きボックス描画(static_cast の繰り返しを減らすため)
    void DrawPanel(float x, float y, float w, float h, int fillColor, int lineColor)
    {
        const int x1 = static_cast<int>(x);
        const int y1 = static_cast<int>(y);
        const int x2 = static_cast<int>(x + w);
        const int y2 = static_cast<int>(y + h);
        DrawBox(x1, y1, x2, y2, fillColor, TRUE);
        DrawBox(x1, y1, x2, y2, lineColor, FALSE);
    }
}

SubMenuMapPage::SubMenuMapPage(SubMenu* owner)
    : m_owner(owner)
    , m_zoom(1.0f)
    , m_scrollOffset(0.0f, 0.0f)
{
}

void SubMenuMapPage::Initialize()
{
    m_zoom = 1.0f;
    m_scrollOffset = Vector2d(0.0f, 0.0f);
}

void SubMenuMapPage::Update(float deltaTime)
{
    if (!m_owner) return;

    const Input& input = m_owner->GetScene()->GetGame()->GetInput();
    const Key& key = input.GetKey();

    // 拡大・縮小 (Q / E)
    if (input.IsDown(Action::ZOOM_IN))
    {
        m_zoom = min(m_zoom + 0.5f * deltaTime, 2.0f);
    }
    if (input.IsDown(Action::ZOOM_OUT));
    {
        m_zoom = max(m_zoom - 0.5f * deltaTime, 0.5f);
    }

    // スクロール (WASD / 矢印)
    const float scrollSpeed = 300.0f * deltaTime;
    if (key.IsDown(Key::W) || key.IsDown(Key::UP))    m_scrollOffset.y += scrollSpeed;
    if (key.IsDown(Key::S) || key.IsDown(Key::DOWN))  m_scrollOffset.y -= scrollSpeed;
    if (key.IsDown(Key::A) || key.IsDown(Key::LEFT))  m_scrollOffset.x += scrollSpeed;
    if (key.IsDown(Key::D) || key.IsDown(Key::RIGHT)) m_scrollOffset.x -= scrollSpeed;
}
void SubMenuMapPage::Draw()
{
    if (!m_owner) return;

    Renderer* renderer = m_owner->GetScene()->GetGame()->GetRenderer();
    if (!renderer) return;
    const std::string& font = m_owner->GetScene()->GetGame()->GatDebugFont();
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_owner->GetScene());

    // メイン領域
    const float leftX = 100.0f;
    const float startY = 130.0f;
    const float width = 1080.0f;
    const float height = 500.0f;

    DrawPanel(leftX, startY, width, height, GetColor(20, 22, 28), GetColor(100, 100, 120));

    // ステージ未入場
    if (!playScene)
    {
        renderer->DrawTextC(
            Vector2d(leftX + width * 0.5f, startY + height * 0.5f),
            "現在ステージに入っていません",
            Color(200, 200, 200), font, 24, false);
        return;
    }

    // マップ描画エリア
    renderer->DrawTextC(
        Vector2d(leftX + width * 0.5f, startY + height * 0.4f),
        "[ ステージ全体マップ表示エリア ]",
        Color(180, 180, 200), font, 22, false);

    // 操作ガイド
    std::string zoomStr = "拡大率: " + std::to_string(static_cast<int>(m_zoom * 100))
        + "%  [Q/E]: 拡大縮小  [WASD/矢印]: スクロール";
    renderer->DrawTextL(Vector2d(leftX + 20.0f, startY + height - 35.0f),
        zoomStr, Color(160, 160, 170), font, 16, false);

    // 右上:状態異常欄
    const float statusX = leftX + width - 260.0f;
    const float statusY = startY + 15.0f;
    DrawPanel(statusX, statusY, 245.0f, 90.0f, GetColor(35, 35, 45), GetColor(120, 120, 140));

    renderer->DrawTextL(Vector2d(statusX + 10.0f, statusY + 8.0f), "【 状態異常 】", Color(255, 215, 0), font, 16, false);
    renderer->DrawTextL(Vector2d(statusX + 15.0f, statusY + 40.0f), "なし (正常)", Color(180, 180, 180), font, 16, false);

    // 右下:アイテム欄
    const float itemX = leftX + width - 260.0f;
    const float itemY = startY + height - 140.0f;
    DrawPanel(itemX, itemY, 245.0f, 95.0f, GetColor(35, 35, 45), GetColor(120, 120, 140));

    renderer->DrawTextL(Vector2d(itemX + 10.0f, itemY + 8.0f), "【 所持アイテム 】", Color(255, 215, 0), font, 16, false);
    renderer->DrawTextL(Vector2d(itemX + 15.0f, itemY + 35.0f), "回復薬 : 0 / 4", Color(220, 220, 220), font, 16, false);
    renderer->DrawTextL(Vector2d(itemX + 15.0f, itemY + 60.0f), "クナイ   : 0", Color(220, 220, 220), font, 16, false);
}