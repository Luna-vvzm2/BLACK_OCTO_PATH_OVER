#include "KaryuTextUI.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"

KaryuTextUI::KaryuTextUI(Scene* scene)
	:UIActor(scene)
{
	m_transform = AddComponent<TransformComponent>();
	m_fireSprite = AddComponent<SpriteComponent>("assets/images/uies/ka.png", false);
	m_dragonSprite = AddComponent<SpriteComponent>("assets/images/uies/ryu.png", false);

	SetName("KaryuTextUI");
}

bool KaryuTextUI::Init() {
	if (!UIActor::Init()) return false;

	m_transform->SetPosition({ 1280.0f / 2, 720.0f / 2 });

	m_fireSprite->SetDrawOffset(-320.0f, -50.0f);
	m_dragonSprite->SetDrawOffset(320.0f, 50.0f);
	m_fireSprite->SetAlpha(0);
	m_dragonSprite->SetAlpha(0);
	return true;
}

void KaryuTextUI::Update(float deltaTime) {
	UIActor::Update(deltaTime);

	m_timer += deltaTime;

	// フェードイン
	if (m_timer < 0.3f)
	{
		m_fireSprite->SetAlpha((int)(m_timer / 0.3f * 255));
	}
	if (m_timer > 0.5f && m_timer < 0.8f)
	{
		m_dragonSprite->SetAlpha((int)((m_timer - 0.5f) / 0.3f * 255));
	}


	// フェードアウト
	if (m_timer > 2.0f)
	{
		float t = (m_timer - 2.0f) / 0.5f;

		m_fireSprite->SetAlpha(255 * (int)(1 - t));
		m_dragonSprite->SetAlpha(255 * (int)(1 - t));
	}


	if (m_timer > 2.5f)
	{
		SetState(State::Dead);
	}
}

void KaryuTextUI::Draw() {
	/*auto transform = GetComponent<TransformComponent>();
	if (!transform) return;

	Vector2d pos = transform->GetPosition();
	float scale = 3.0f;

	m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
		pos,
		scale, scale,
		0.0f,
		m_fireSprite->GetHandle(),
		true
	);

	m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
		pos,
		scale, scale,
		0.0f,
		m_dragonSprite->GetHandle(),
		true
	);*/
	UIActor::Draw();
}