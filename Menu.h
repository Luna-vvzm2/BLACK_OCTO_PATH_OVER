#pragma once

#include "MapMenu.h"
#include "StatusMenu.h"
#include "NewsMenu.h"
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

    enum class Tab
    {
        Map = 0,
        Status,
        News,
        Count
    };

    Game* GetGame() const;
    PlayerEntity* GetPlayer() const;

    bool IsOpen() const;
    void Toggle();

    Tab GetCurrentTab() const;

    MenuCursor& GetCursor()
    {
        return m_cursor;
    }

private:
    void DrawTabs();
    void ChangeTab(int delta);

    PlayScene* m_owner;
    MenuCursor m_cursor;

    MapMenu    m_mapMenu;
    StatusMenu m_statusMenu;
    NewsMenu   m_newsMenu;

    Tab m_currentTab = Tab::Map;

    bool m_isOpen = false;
};