#include "TitleUI.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"

TitleUI::TitleUI(Scene* scene)
	: UIActor(scene)
{
	m_transform = AddComponent<TransformComponent>();
	m_title = AddComponent<SpriteComponent>("assets/images/title/titletest.png", false);
	if (!m_title) {
		std::cerr << "画像読み込み失敗\n";
	}
	SetName("HPBarUI");

}
bool TitleUI::Init() {
	if (!UIActor::Init()) return false;

	m_transform->SetPosition({ 0, 0 });

	return true;
}

void TitleUI::Update(float deltaTime) {
	UIActor::Update(deltaTime);

}

void TitleUI::Draw() {
	if (!m_title) return;

	auto transform = GetComponent<TransformComponent>();
	if (!transform) return;

	Vector2d pos = transform->GetPosition();
	if (m_title->GetHandle() < 0) return;
	int w = 0, h = 0;
	if (GetGraphSize(m_title->GetHandle(), &w, &h) < 0 || w <= 0 || h <= 0) return;

	const Game* game = m_scene->GetGame();
	const float scaleX = static_cast<float>(game->GetWidth()) / w;
	const float scaleY = static_cast<float>(game->GetHeight()) / h;

	// スクリーン座標描画（カメラ影響なし）
	m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
		pos, scaleX, scaleY, 0.0f, m_title->GetHandle(),
		false, Vector2d(0, 0), 255, false, false, false
	);
}

void TitleUI::SetPosition(float x, float y)
{
	if (m_transform)
		m_transform->SetPosition({ x, y });
}