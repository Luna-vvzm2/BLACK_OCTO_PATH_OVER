#pragma once

#include <string>

class Game;

namespace MenuVisual
{
    void DrawOption(Game* game, const std::string& label, float x, float y,
        int fontSize, bool selected, bool enabled = true);
}

// Shared settings UI. It can be opened from the title or from a pause menu.
class SettingsMenu
{
public:
    explicit SettingsMenu(Game* game, bool overGame = false);

    void Open();
    bool IsOpen() const { return m_open; }
    void Update();
    void Draw() const;

private:
    enum class Page { Root, Volume, Controls };

    Game* m_game;
    bool m_overGame = false;
    Page m_page = Page::Root;
    bool m_open = false;
    int m_cursor = 0;
};

class PauseMenu
{
public:
    enum class Result { None, Resume, StageSelect, Quit };

    PauseMenu(Game* game, bool fromStageSelect);

    void Open();
    bool IsOpen() const { return m_open; }
    void Toggle();
    Result Update();
    void Draw(int alpha = 255) const;

private:
    Game* m_game;
    bool m_fromStageSelect;
    bool m_open = false;
    int m_cursor = 0;
    SettingsMenu m_settings;
};
