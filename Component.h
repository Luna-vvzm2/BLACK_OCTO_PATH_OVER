#pragma once
#include <string>

class Actor;

class Component
{
public:
	explicit Component(Actor* owner);
	virtual ~Component();

	virtual bool Init() { return true; }
	virtual void Update(float deltaTime) {}
	virtual void Draw() {}


protected:

	Actor* m_owner;

};