#pragma once

#include "Component.h"

class AlertGaugeComponent : public Component
{
public:
    AlertGaugeComponent(Actor* owner);

    void SetGauge(float value);
    void AddGauge(float value);

    float GetGauge() const;

    bool IsDefenseless() const;
    bool IsAlert() const;
    bool IsCombat() const;

    void Draw() override;

private:
    float m_gauge;

    static constexpr float GAUGE_MIN = -100.0f;
    static constexpr float GAUGE_MAX = 100.0f;
};