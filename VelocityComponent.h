#pragma once
#include "Component.h"
#include "Vector2d.h"

class VelocityComponent : public Component {
public:
	explicit VelocityComponent(Actor* owner);
	~VelocityComponent() override = default;

	void SetVelocity(const Vector2d& vel) { m_velocity = vel; }
	const Vector2d& GetVelocity() const { return m_velocity; }

	void Update(float deltaTime) override;

	void Set(const Vector2d& v) { m_velocity = v; }
	void SetX(const float& v) { m_velocity.x = v; }
	const Vector2d& Get() const { return m_velocity; }

private:
	Vector2d m_velocity;
};