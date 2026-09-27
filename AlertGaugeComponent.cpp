#include "AlertGaugeComponent.h"
#include <DxLib.h>
#include "Actor.h"
#include "EntityActor.h"
#include "Game.h"
#include "Scene.h"
#include "Renderer.h"

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

    // ƒQ[ƒW‚Ì‘å‚«‚³
    const float gaugeWidth = 80.0f;
    const float gaugeHeight = 8.0f;

    // “G‚Ì“ªã
    Vector2d gaugePos(
        pos.x,
        pos.y - 60.0f
    );

    // -100 ` 100 ‚ğ 0.0 ` 1.0 ‚É•ÏŠ·
    float rate = m_gauge / GAUGE_MAX;

    if (rate < 0.0f)
    {
        rate = 0.0f;
    }

    if (rate > 1.0f)
    {
        rate = 1.0f;
    }
    // ”wŒi
    renderer->DrawRectCenter(
        gaugePos,
        gaugeWidth,
        gaugeHeight,
        Color(80, 80, 80),
        true,
        true
    );

    if (rate <= 0.0f)
    {
        return;
    }

    // ƒQ[ƒWF
    Color gaugeColor;

    if (IsDefenseless())
    {
        // –³–h”õFÂ
        gaugeColor = Color(0, 100, 255);
    }
    else if (IsCombat())
    {
        // í“¬FƒIƒŒƒ“ƒW
        gaugeColor = Color(255, 150, 0);
    }
    else
    {
        // Œx‰úFŠDF
        gaugeColor = Color(180, 180, 180);
    }

    // ¶‘¤‚©‚çƒQ[ƒW‚ğL‚Î‚·
    float fillWidth = gaugeWidth * rate;

    Vector2d fillPos(
        gaugePos.x - (gaugeWidth - fillWidth) * 0.5f,
        gaugePos.y
    );

    renderer->DrawRectCenter(
        fillPos,
        fillWidth,
        gaugeHeight,
        gaugeColor,
        true,
        true
    );
}