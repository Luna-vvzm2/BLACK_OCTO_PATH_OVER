#include "Component.h"

Component::Component(Actor* owner)
	: m_owner(owner)
{
}

Component::~Component() = default;
