#pragma once
#include "UIActor.h"

class TransformComponent;
class SpriteComponent;

class TitleUI : public UIActor
{
public:
	TitleUI(Scene* scene);
	virtual ~TitleUI() = default;

	virtual bool Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;

	void SetPosition(float x, float y);

private:
	TransformComponent* m_transform = nullptr;
	SpriteComponent* m_sprite = nullptr;
	SpriteComponent* m_title = nullptr;

};

