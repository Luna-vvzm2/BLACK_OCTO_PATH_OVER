#pragma once
#include "Component.h"
#include "Vector2d.h"

class GravityComponent : public Component {

public:
	explicit GravityComponent(Actor* owner, float gravity = 500.0f);
	~GravityComponent() override = default;

	void Update(float deltaTime) override;

	void SetGravity(float g) { m_gravity = g; }
	float GetGravity() const { return m_gravity; }

private:
	float m_gravity;
};

