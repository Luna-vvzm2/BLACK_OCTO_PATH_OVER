#pragma once
#include "UIActor.h"
#include "Vector2d.h"
#include "Color.h"

class PlayScene;

// ゲームオーバー選択メニューUI
class GameOverMenuUI : public UIActor {
public:
    enum class MenuItem {
        CONTINUE = 0,
        WORLD_MAP,
        TITLE,
        ITEM_COUNT
    };

    explicit GameOverMenuUI(Scene* scene);
    ~GameOverMenuUI() override = default;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    bool IsActive() const { return m_isActive; }
    void SetActive(bool active) { m_isActive = active; }

    MenuItem GetSelectedItem() const { return m_selectedItem; }
    bool IsDecided() const { return m_isDecided; }
    void ResetDecided() { m_isDecided = false; }

private:
    bool m_isActive = false;
    bool m_isDecided = false;
    MenuItem m_selectedItem = MenuItem::CONTINUE;

    // メニュー項目の表示位置情報
    struct MenuItemRect {
        Vector2d pos;
        float width;
        float height;
    };
    MenuItemRect m_menuRects[static_cast<int>(MenuItem::ITEM_COUNT)];

    // マウスホバー判定
    bool IsMouseOverMenuItem(int itemIndex, int mouseX, int mouseY) const;
    void UpdateMenuItemRects();
};
