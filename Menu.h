#pragma once

#include "MapMenu.h"
#include "StatusMenu.h"
#include "NewsMenu.h"
#include "SkillMenu.h"
#include "EquipmentMenu.h"
#include "MenuCursor.h"

class PlayScene;
class PlayerEntity;
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
    PlayerEntity* GetPlayer() const;

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
    void DrawTabs() const;
    void ChangeTab(int delta);

    PlayScene* m_owner;
    MenuCursor m_cursor;

    SkillMenu m_skillMenu;
    EquipmentMenu m_equipmentMenu;

    MapMenu    m_mapMenu;
    StatusMenu m_statusMenu;
    NewsMenu   m_newsMenu;

    int m_tabCursor = 0;
    int m_lockedSkillIcon = -1;

    MenuCursorArea m_cursorArea = MenuCursorArea::Tab;

    enum class Tab
    {
        Map = 0,
        Status,
        News,
        Count
    };

    Tab m_currentTab = Tab::Map;

    bool m_isOpen = false;
};