#include "ShurikenUI.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Renderer.h"

ShurikenUI::ShurikenUI(Scene* scene, float x, float y)
    : UIActor(scene)
{
    SetPosition(x, y);

    // 小さな手裏剣アイコンを読み込む
    m_icon = AddComponent<SpriteComponent>("assets/images/uies/kunai.png", false);
}

void ShurikenUI::SetCount(int count)
{
    m_count = count;
}

void ShurikenUI::Draw()
{
    if (!m_isVisible) return;

    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    const std::string& font = m_scene->GetGame()->GatDebugFont();

    // --- ① 手裏剣アイコンを描画 ---
    float scale = 0.4f; // 小さめ
    Vector2d iconPos(GetX(), GetY());

    renderer->DrawSpriteEx(
        iconPos,
        scale, scale,
        0.0f,
        m_icon->GetHandle(),
        true,
        Vector2d(0, 0),
        255,
        false, false, false
    );

    // --- ② 数字を描画（アイコンの右に表示） ---
    std::string text = std::to_string(m_count);

    renderer->DrawTextL(
        Vector2d(GetX() + 40, GetY() + 10), // アイコンの右に配置
        text,
        Color(255, 255, 255),
        font,
        32,
        false
    );
}
