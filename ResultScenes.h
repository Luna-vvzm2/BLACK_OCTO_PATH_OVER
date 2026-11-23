#pragma once
#include "Scene.h"
#include "GameOverScene.h"

// New presentation only. Existing result scenes and retry logic stay unchanged.
class ClearResultScene final : public Scene
{
public:
    ClearResultScene(Game* game, float clearTime);
    ~ClearResultScene() override;
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
private:
    float m_clearTime = 0.0f;
    float m_inputDelay = 0.3f;
    int m_art = -1;
    int m_background = -1;
    bool m_captured = false;
};

class GameOverResultScene final : public GameOverScene
{
public:
    explicit GameOverResultScene(Game* game);
    ~GameOverResultScene() override;
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
private:
    float m_inputDelay = 0.3f;
    int m_art = -1;
};
