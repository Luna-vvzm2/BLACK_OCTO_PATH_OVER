#pragma once
#include "Component.h"
#include "Vector2d.h"

class TransformComponent : public Component {
public:
	explicit TransformComponent(Actor* owner);
	~TransformComponent() override = default;

	void SetPosition(const Vector2d& pos) { m_position = pos; }
	const Vector2d& GetPosition() const { return m_position; }

	void SetScale(Vector2d scale) { m_scale = scale; }
	const Vector2d& GetScale() const { return m_scale; }

	void SetAngle(float angle) { m_angle = angle; }
	float GetAngle() const { return m_angle; }

private:
	Vector2d m_position;
	Vector2d m_scale;
	float m_angle;
};