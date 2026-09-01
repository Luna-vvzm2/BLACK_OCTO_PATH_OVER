#pragma once
#include "UIActor.h"

class SpriteComponent;

class ShurikenUI : public UIActor
{
public:
    ShurikenUI(Scene* scene, float x, float y);
    void SetCount(int count);
    void Draw() override;

    void SetVisible(bool visible) { m_isVisible = visible; }
    bool IsVisible() const { return m_isVisible; }
private:
    SpriteComponent* m_icon = nullptr; // è¨Ç≥Ç»éËó†åïÉAÉCÉRÉì
    int m_count = 0;
    bool m_isVisible = true;
};
