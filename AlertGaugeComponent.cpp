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

    // ゲージの大きさ
    const float gaugeWidth = 80.0f;
    const float gaugeHeight = 8.0f;

    // 敵の頭上
    Vector2d gaugePos(
        pos.x,
        pos.y - 60.0f
    );

    // ゲージの背景
    renderer->DrawRectCenter(
        gaugePos,
        gaugeWidth,
        gaugeHeight,
        Color(80, 80, 80),
        true,
        true
    );

    // -100 ～ 100 の値を
    // 0 ～ 1 のゲージ割合に変換
    float rate = std::abs(m_gauge) / GAUGE_MAX;

    if (rate > 1.0f)
    {
        rate = 1.0f;
    }

    // 0の場合はゲージを表示しない
    if (rate <= 0.0f)
    {
        return;
    }

    // ゲージの色
    Color gaugeColor;

    if (m_gauge >= 0.0f)
    {
        // 0以上：オレンジ
        gaugeColor = Color(255, 150, 0);
    }
    else
    {
        // 0未満：青
        gaugeColor = Color(0, 100, 255);
    }

    // ゲージの表示幅
    float fillWidth = gaugeWidth * rate;

    // 左端をゲージの左端に合わせる
    Vector2d fillPos(
        gaugePos.x - (gaugeWidth - fillWidth) * 0.5f,
        gaugePos.y
    );

    // 左から右へゲージを表示
    renderer->DrawRectCenter(
        fillPos,
        fillWidth,
        gaugeHeight,
        gaugeColor,
        true,
        true
    );
}