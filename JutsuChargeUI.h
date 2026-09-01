#pragma once
#include "UIActor.h"

class SpriteComponent;

class JutsuChargeUI : public UIActor
{
public:
    explicit JutsuChargeUI(Scene* scene);
    virtual ~JutsuChargeUI() = default;

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void SetNinImagePosition(float x, float y);
    void SetNinImageScale(float scale);

    void SetVisible(bool visible) { m_isVisible = visible; }
private:
    SpriteComponent* m_ninImage = nullptr;

	bool m_jutsuchage = false;

    float m_ninImageX = 10.0f;
    float m_ninImageY = 20.0f;
    float m_ninImageScale = 1.0f;
    int m_damageCount = 0;
    bool m_isVisible = true;
};