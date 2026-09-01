#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "Actor.h"

VelocityComponent::VelocityComponent(Actor* owner)
	: Component(owner)
	, m_velocity(Vector2d::Zero())
{
}

void VelocityComponent::Update(float deltaTime)
{
	if (!m_owner) return;

	auto transform = m_owner->GetComponent<TransformComponent>();
	if (transform)
		transform->SetPosition(transform->GetPosition() + m_velocity * deltaTime);

}