#pragma once
#include "UIActor.h"

class TransformComponent;
class SpriteComponent;

class KaryuTextUI : public UIActor
{
public:
    KaryuTextUI(Scene* scene);
    ~KaryuTextUI() = default;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    TransformComponent* m_transform = nullptr;
    SpriteComponent* m_fireSprite;
    SpriteComponent* m_dragonSprite;

    float m_timer = 0.0f;
};

