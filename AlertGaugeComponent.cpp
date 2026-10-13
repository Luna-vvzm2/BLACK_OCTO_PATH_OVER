#include "AlertGaugeComponent.h"
#include <DxLib.h>
#include "Actor.h"
#include "EntityActor.h"
#include "Game.h"
#include "Scene.h"
#include "Renderer.h"
#include <cmath>

AlertGaugeComponent::AlertGaugeComponent(Actor* owner)
    : Component(owner)
    , m_gauge(0.0f)
{
}

void AlertGaugeComponent::SetGauge(float value)
{
    if (value < GAUGE_MIN)
    {
        m_gauge = GAUGE_MIN;
    }
    else if (value > GAUGE_MAX)
    {
        m_gauge = GAUGE_MAX;
    }
    else
    {
        m_gauge = value;
    }
}

void AlertGaugeComponent::AddGauge(float value)
{
    SetGauge(m_gauge + value);
}

float AlertGaugeComponent::GetGauge() const
{
    return m_gauge;
}

bool AlertGaugeComponent::IsDefenseless() const
{
    return m_gauge <= 0.0f;
}

bool AlertGaugeComponent::IsAlert() const
{
    return m_gauge > 0.0f &&
        m_gauge < 100.0f;
}

bool AlertGaugeComponent::IsCombat() const
{
    return m_gauge >= 100.0f;
}

void AlertGaugeComponent::Draw()
{
    if (m_owner == nullptr)
    {
        return;
    }

    EntityActor* entityActor =
        dynamic_cast<EntityActor*>(m_owner);

    if (entityActor == nullptr)
    {
        return;
    }

    Scene* scene = entityActor->GetScene();

    if (scene == nullptr || scene->GetGame() == nullptr)
    {
        return;
    }

    Renderer* renderer = scene->GetGame()->GetRenderer();

    if (renderer == nullptr)
    {
        return;
    }

    Vector2d pos = entityActor->GetPos();

    // ÉQÅ[ÉWÇÃëÂÇ´Ç≥
    const float gaugeWidth = 80.0f;
    const float gaugeHeight = 8.0f;

    // ìGÇÃì™è„
    Vector2d gaugePos(
        pos.x,
        pos.y - 60.0f
    );

    // ÉQÅ[ÉWÇÃîwåi
    renderer->DrawRectCenter(
        gaugePos,
        gaugeWidth,
        gaugeHeight,
        Color(80, 80, 80),
        true,
        true
    );

    // -100 Å` 100 ÇÃílÇ
    // 0 Å` 1 ÇÃÉQÅ[ÉWäÑçáÇ…ïœä∑
    float rate = std::abs(m_gauge) / GAUGE_MAX;

    if (rate > 1.0f)
    {
        rate = 1.0f;
    }

    // 0ÇÃèÍçáÇÕÉQÅ[ÉWÇï\é¶ÇµÇ»Ç¢
    if (rate <= 0.0f)
    {
        return;
    }

    // ÉQÅ[ÉWÇÃêF
    Color gaugeColor;

    if (m_gauge >= 70.0f)
    {
        // 70Å`100ÅFê‘
        gaugeColor = Color(255, 0, 0);
    }
    else if (m_gauge >= 0.0f)
    {
        // 0Å`69ÅFÉIÉåÉìÉW
        gaugeColor = Color(255, 150, 0);
    }
    else
    {
        // -100Å`0ñ¢ñûÅFê¬
        gaugeColor = Color(0, 100, 255);
    }

    // ÉQÅ[ÉWÇÃï\é¶ïù
    float fillWidth = gaugeWidth * rate;

    // ç∂í[ÇÉQÅ[ÉWÇÃç∂í[Ç…çáÇÌÇπÇÈ
    Vector2d fillPos(
        gaugePos.x - (gaugeWidth - fillWidth) * 0.5f,
        gaugePos.y
    );

    // ç∂Ç©ÇÁâEÇ÷ÉQÅ[ÉWÇï\é¶
    renderer->DrawRectCenter(
        fillPos,
        fillWidth,
        gaugeHeight,
        gaugeColor,
        true,
        true
    );
}