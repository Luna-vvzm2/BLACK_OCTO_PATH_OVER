#include "GameOverMenuUI.h"
#include "Scene.h"
#include "Game.h"
#include "Input.h"
#include "Renderer.h"
#include "TransformComponent.h"

GameOverMenuUI::GameOverMenuUI(Scene* scene)
    : UIActor(scene)
{
}

bool GameOverMenuUI::Init() {
    if (!UIActor::Init()) return false;
    m_isActive = false;
    m_isDecided = false;
    m_selectedItem = MenuItem::CONTINUE;
    return true;
}

void GameOverMenuUI::UpdateMenuItemRects() {
    float screenWidth = static_cast<float>(m_scene->GetGame()->GetWidth());
    float screenHeight = static_cast<float>(m_scene->GetGame()->GetHeight());
    float menuStartY = screenHeight / 2.0f;
    float itemHeight = 80.0f;
    float textWidth = 400.0f;
    float textHeight = 50.0f;

    // コンティニューの位置情報
    m_menuRects[0].pos = Vector2d(screenWidth / 2.0f - 230, menuStartY);
    m_menuRects[0].width = textWidth;
    m_menuRects[0].height = textHeight;

    // ワールドマップへ戻るの位置情報
    m_menuRects[1].pos = Vector2d(screenWidth / 2.0f - 180, menuStartY + itemHeight);
    m_menuRects[1].width = textWidth;
    m_menuRects[1].height = textHeight;

    // タイトル画面に戻るの位置情報
    m_menuRects[2].pos = Vector2d(screenWidth / 2.0f - 180, menuStartY + (2 * itemHeight));
    m_menuRects[2].width = textWidth;
    m_menuRects[2].height = textHeight;
}

bool GameOverMenuUI::IsMouseOverMenuItem(int itemIndex, int mouseX, int mouseY) const {
    if (itemIndex < 0 || itemIndex >= static_cast<int>(MenuItem::ITEM_COUNT)) return false;

    const MenuItemRect& rect = m_menuRects[itemIndex];
    return mouseX >= rect.pos.x && mouseX <= rect.pos.x + rect.width &&
        mouseY >= rect.pos.y && mouseY <= rect.pos.y + rect.height;
}

void GameOverMenuUI::Update(float deltaTime) {
    if (!m_isActive) return;

    const Input& input = m_scene->GetGame()->GetInput();

    // 上下キーで選択
    if (input.IsTrigger(Action::UP)) {
        m_selectedItem = MenuItem((static_cast<int>(m_selectedItem) - 1 + static_cast<int>(MenuItem::ITEM_COUNT)) % static_cast<int>(MenuItem::ITEM_COUNT));
    }
    if (input.IsTrigger(Action::DOWN)) {
        m_selectedItem = MenuItem((static_cast<int>(m_selectedItem) + 1) % static_cast<int>(MenuItem::ITEM_COUNT));
    }

    // Enterキーで決定
    if (input.IsTrigger(Action::ENTER)) {
        m_isDecided = true;
    }

    // マウス入力の処理
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    UpdateMenuItemRects();

    // マウスホバー判定
    for (int i = 0; i < static_cast<int>(MenuItem::ITEM_COUNT); i++) {
        if (IsMouseOverMenuItem(i, mouseX, mouseY)) {
            m_selectedItem = MenuItem(i);

            // マウスクリック判定
            if (GetMouseInput() & MOUSE_INPUT_LEFT) {
                m_isDecided = true;
            }
        }
    }
}

void GameOverMenuUI::Draw() {
    if (!m_isActive) return;

    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    const std::string& debugFont = m_scene->GetGame()->GatDebugFont();

    // 半透明の背景（黒）
    renderer->DrawRect(
        Vector2d(0, 0),
        static_cast<float>(m_scene->GetGame()->GetWidth()),
        static_cast<float>(m_scene->GetGame()->GetHeight()),
        Color(0, 0, 0),
        true,
        false
    );

    // タイトル
    Vector2d titlePos(m_scene->GetGame()->GetWidth() / 2.0f - 100, 100);
    renderer->DrawTextL(titlePos, "GAME OVER", Color(255, 0, 0), debugFont, 48, false);

    // メニュー項目
    const char* items[] = { "コンティニュー", "ワールドマップへ戻る", "タイトル画面に戻る" };
    float menuStartY = m_scene->GetGame()->GetHeight() / 2.0f;
    float itemHeight = 80.0f;

    for (int i = 0; i < static_cast<int>(MenuItem::ITEM_COUNT); i++) {
        Vector2d itemPos(m_scene->GetGame()->GetWidth() / 2.0f - 150, menuStartY + (i * itemHeight));

        // 選択中の項目はハイライト
        if (i == static_cast<int>(m_selectedItem)) {
            renderer->DrawTextL(itemPos, "> " + std::string(items[i]) + " <", Color(255, 0, 0), debugFont, 32, false);
        }
        else {
            renderer->DrawTextL(itemPos, items[i], Color(200, 200, 200), debugFont, 32, false);
        }
    }
}
