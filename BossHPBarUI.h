#pragma once

#include "UIActor.h"

class EnemyEntity;

// Screen-space HP gauge for the current boss encounter.
class BossHPBarUI : public UIActor
{
public:
    explicit BossHPBarUI(Scene* scene);
    ~BossHPBarUI() override;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    bool IsVisible() const { return m_visible; }
    void Reset();

private:
    EnemyEntity* m_activeBoss = nullptr; // Compared with scene actors; never dereferenced after removal.
    int m_frameHandle = -1;
    float m_displayRatio = 1.0f;
    bool m_visible = false;
};
