#pragma once

#include "Scene.h"
#include "ScreenMenus.h"

class StageSelectScene : public Scene
{
public:
    explicit StageSelectScene(Game* game);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    PauseMenu m_pauseMenu;
};
