#include "BackGroundUI.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "PlayScene.h"

BackGroundUI::BackGroundUI(Scene* scene, const std::string& texturePath)
	: UIActor(scene)
{
	m_transform = AddComponent<TransformComponent>();
	m_sprite = AddComponent<SpriteComponent>(texturePath);
	if (!m_sprite) {
		std::cerr << "‰æ‘œ“Ç‚Ýž‚ÝŽ¸”s\n";
	}
	SetName("HPBarUI");
}

bool BackGroundUI::Init()
{
	if (!UIActor::Init()) return false;

	m_transform->SetPosition({ 0, 5000 });

	return true;
}
void BackGroundUI::Update(float deltaTime) {
	UIActor::Update(deltaTime);

}

void BackGroundUI::Draw() {
	if (!m_sprite || !m_transform) return;

	Vector2d basePos = m_transform->GetPosition();

	auto* playScene = dynamic_cast<PlayScene*>(m_scene);
	if (!playScene) return;

	Camera& camera = playScene->GetCamera();

	Game* game = m_scene->GetGame();

	Vector2d screenCenter(
		game->GetWidth() / 2.0f,
		game->GetHeight() / 2.0f
	);

	// ƒJƒƒ‰’†S‚Æ‚Ì·•ª
	Vector2d camOffset = camera.GetCenter() - screenCenter;

	float parallax = 0.2f;  // š ‰œs‚«ŒW”

	Vector2d drawPos = basePos - camOffset * parallax;

	int scale = 2;

	//	ƒJƒƒ‰–³Ž‹ifalsej‚Å•`‰æ
	game->GetRenderer()->DrawSpriteEx(
		drawPos,
		(float)scale, (float)scale,
		0.0f,
		m_sprite->GetHandle(),
		true,
		Vector2d(0, 0),
		255,
		false, false, false
	);
}

void BackGroundUI::SetPosition(float x, float y)
{
	if (m_transform)
		m_transform->SetPosition({ x, y });
}