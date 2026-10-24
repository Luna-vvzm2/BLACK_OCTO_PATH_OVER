#include "ShurikenUI.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Renderer.h"

ShurikenUI::ShurikenUI(Scene* scene, float x, float y)
    : UIActor(scene)
{
    SetPosition(x, y);

    // ¬‚³‚Èè— Œ•ƒAƒCƒRƒ“‚ğ“Ç‚İ‚Ş
    m_icon = AddComponent<SpriteComponent>("assets/images/icon/kunaiItem.png", false);
}

void ShurikenUI::SetCount(int count)
{
    m_count = count;
}

void ShurikenUI::Draw()
{
    if (!m_isVisible || !m_icon || m_icon->GetHandle() < 0) return;

    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    const std::string& font = m_scene->GetGame()->GatDebugFont();
    const int x = static_cast<int>(GetX());
    const int y = static_cast<int>(GetY());
    const int itemSize = 78;

    DrawExtendGraph(x, y, x + itemSize, y + itemSize, m_icon->GetHandle(), TRUE);

    const std::string count = std::to_string(m_count);
    renderer->DrawTextL(Vector2d(GetX() + 60.0f, GetY() + 15.0f),
        count, Color(20, 20, 20), font, 38, false);
    renderer->DrawTextL(Vector2d(GetX() + 58.0f, GetY() + 13.0f),
        count, Color(255, 255, 255), font, 38, false);
}