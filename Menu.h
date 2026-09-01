#pragma once

#include "SkillMenu.h"
#include "EquipmentMenu.h"
#include "MenuCursor.h"

class PlayScene;
class Game;

class Menu
{
public:
    explicit Menu(PlayScene* owner);

    void Initialize();

    void Update(float deltaTime);

    void Draw();

    enum class MenuCursorArea
    {
        Tab,
        SkillList,
        EquipmentList
    };

    Game* GetGame() const;

    bool IsOpen() const;
    void Toggle();

    MenuCursorArea GetCursorArea() const;

    int GetTabCursor() const;

    int GetLockedSkillIcon() const;

    MenuCursor& GetCursor()
    {
        return m_cursor;
    }

    void SetCursorArea(MenuCursorArea area);

private:

    void DrawTabs();
    MenuCursor m_cursor;

    PlayScene* m_owner;

    SkillMenu m_skillMenu;
    EquipmentMenu m_equipmentMenu;

    int m_tabCursor = 0;
    int m_lockedSkillIcon = -1;

    MenuCursorArea m_cursorArea = MenuCursorArea::Tab;

    enum class Tab
    {
        Skill,
        Equipment
    };

    Tab m_currentTab = Tab::Skill;

    bool m_isOpen = false;



};