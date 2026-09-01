#include "TransformComponent.h"

TransformComponent::TransformComponent(Actor* owner)
	: Component(owner)
	, m_position(Vector2d::Zero())
	, m_scale({ 1.0f, 1.0f })
	, m_angle(0.0f)
{
}