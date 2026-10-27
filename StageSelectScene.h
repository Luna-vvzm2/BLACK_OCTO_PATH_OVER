#pragma once

#include "Scene.h"

class PlayScene;

class StageSelectScene : public Scene
{
public:
    explicit StageSelectScene(Game* game, PlayScene* previousPlayScene = nullptr);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    PlayScene* m_previousPlayScene = nullptr;
};
