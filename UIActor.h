#pragma once
#include "Actor.h"

class UIActor : public Actor
{
public:
	UIActor(Scene* scene);
	virtual ~UIActor() = default;

	//	UIは画面両面のため、Actorの位置は画面座標として扱う
	virtual bool Init() override { return true; }
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;

	//	ActorTypeをUIで取得
	virtual ActorType GetType() const override { return ActorType::UI; }

	// UI の座標（画面座標）
	void SetPosition(float x, float y) { m_posX = x; m_posY = y; }
	float GetX() const { return m_posX; }
	float GetY() const { return m_posY; }

protected:
	float m_posX;
	float m_posY;
};

