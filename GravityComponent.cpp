#include "GravityComponent.h"
#include "VelocityComponent.h"
#include "Actor.h"

constexpr float MAX_FALL_SPEED = 1000.0f;

GravityComponent::GravityComponent(Actor* owner, float gravity)
	: Component(owner),
	m_gravity(gravity)
{
}

void GravityComponent::Update(float deltaTime) {

	if (!m_owner) return;

	auto velocity = m_owner->GetComponent<VelocityComponent>();
	if (velocity) {
		auto vel = velocity->Get();
		vel.y += m_gravity * deltaTime;

		if (vel.y > MAX_FALL_SPEED)
		{
			vel.y = MAX_FALL_SPEED;
		}
		velocity->Set(vel);
	}

}