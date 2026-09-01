#pragma once
#include "UIActor.h"

class TransformComponent;
class SpriteComponent;

class BackGroundUI : public UIActor
{
public:
	BackGroundUI(Scene* scene, const std::string& texturePath);
	virtual ~BackGroundUI() = default;

	virtual bool Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;

	void SetPosition(float x, float y);

private:
	TransformComponent* m_transform = nullptr;
	SpriteComponent* m_sprite = nullptr;

};

