#pragma once
#include "MenuPage.h"

#include <vector>

class EquipmentMenu : public MenuPage
{
public:
    explicit EquipmentMenu(Menu* owner);

    void Initialize() override;
    void Draw() override;
    void Update(float deltaTime) override;

protected:
    void ClampCursor() override;

private:

    void DrawNinjutsuSlots();
    void DrawGofuSlots();
    void DrawKatanaSlot();
    void DrawEquipmentInventory();
    void DrawEquipmentDescription();

    bool m_inventoryMode = false;

    int m_prevCategory = 0;
    int m_prevIndex = 0;

    Vector2d GetCursorPosition() const;
};