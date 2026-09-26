#include "PlayerStatusUI.h"

#include "PlayerEntity.h"
#include "Game.h"
#include "Renderer.h"

#include <DxLib.h>

PlayerStatusUI::PlayerStatusUI(
	Scene* scene,
	PlayerEntity* player,
	float x,
	float y)
	: UIActor(scene)
	, m_player(player)
	, m_x(x)
	, m_y(y)
{
}

bool PlayerStatusUI::Init()
{
	if (!UIActor::Init())
	{
		return false;
	}

	return true;
}

void PlayerStatusUI::Update(float deltaTime)
{
	UIActor::Update(deltaTime);
}

void PlayerStatusUI::Draw()
{
	if (!m_player)
	{
		return;
	}

	Game* game = m_scene->GetGame();

	if (!game)
	{
		return;
	}

	Renderer* renderer = game->GetRenderer();

	if (!renderer)
	{
		return;
	}

	const std::string& font = game->GatDebugFont();

	// =========================
	// îwåiÉpÉlÉã
	// =========================

	const int panelX = static_cast<int>(m_x);
	const int panelY = static_cast<int>(m_y);

	const int panelWidth = 300;
	const int panelHeight = 115;

	DrawBox(
		panelX,
		panelY,
		panelX + panelWidth,
		panelY + panelHeight,
		GetColor(20, 20, 20),
		TRUE
	);

	DrawBox(
		panelX,
		panelY,
		panelX + panelWidth,
		panelY + panelHeight,
		GetColor(255, 255, 255),
		FALSE
	);

	// =========================
	// âÒïúÉAÉCÉeÉÄ
	// =========================

	const int healCount = m_player->GetHealItem();

	renderer->DrawTextL(
		Vector2d(
			m_x + 10.0f,
			m_y + 8.0f
		),
		"Heal x" + std::to_string(healCount),
		Color(255, 255, 255),
		font,
		24,
		false
	);

	// =========================
	// îEèp
	// =========================

	const char* jutsuNames[4] =
	{
		"Tako",
		"Tora",
		"Kaeru",
		"Shachi"
	};

	const float slotSize = 62.0f;
	const float slotGap = 8.0f;

	for (int i = 0; i < 4; ++i)
	{
		float slotX =
			m_x + 8.0f +
			i * (slotSize + slotGap);

		float slotY =
			m_y + 42.0f;

		bool own = m_player->GetOwnJutsu(i);

		int fillColor;

		if (own)
		{
			fillColor = GetColor(220, 220, 220);
		}
		else
		{
			fillColor = GetColor(60, 60, 60);
		}

		DrawBox(
			static_cast<int>(slotX),
			static_cast<int>(slotY),
			static_cast<int>(slotX + slotSize),
			static_cast<int>(slotY + slotSize),
			fillColor,
			TRUE
		);

		DrawBox(
			static_cast<int>(slotX),
			static_cast<int>(slotY),
			static_cast<int>(slotX + slotSize),
			static_cast<int>(slotY + slotSize),
			GetColor(255, 255, 255),
			FALSE
		);

		int textColor;

		if (own)
		{
			textColor = GetColor(0, 0, 0);
		}
		else
		{
			textColor = GetColor(140, 140, 140);
		}

		renderer->DrawTextL(
			Vector2d(
				slotX + 5.0f,
				slotY + 19.0f
			),
			jutsuNames[i],
			Color(
				own ? 0 : 140,
				own ? 0 : 140,
				own ? 0 : 140
			),
			font,
			16,
			false
		);
	}
}