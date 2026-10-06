#pragma once

class Game;

// Shared settings UI. It can be opened from the title or from a pause menu.
class SettingsMenu
{
public:
    explicit SettingsMenu(Game* game);

    void Open();
    bool IsOpen() const { return m_open; }
    void Update();
    void Draw() const;

private:
    enum class Page { Root, Volume, Controls };

    Game* m_game;
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
    Result Update();
    void Draw() const;

private:
    Game* m_game;
    bool m_fromStageSelect;
    bool m_open = false;
    int m_cursor = 0;
    SettingsMenu m_settings;
};
