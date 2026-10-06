#include "PlayerStatusUI.h"

#include "PlayerEntity.h"
#include "Game.h"
#include "Renderer.h"
#include <string>
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

	m_jutsuIconHandle =
		LoadGraph("assets/images/uies/ninjutu_icon_sheet.png");

	if (m_jutsuIconHandle == -1)
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
	// 背景パネル
	// =========================

	const int panelX = static_cast<int>(m_x);
	const int panelY = static_cast<int>(m_y);

	const int panelWidth = 300;
	const int panelHeight = 215;

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
	// 回復アイテム
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
// 忍術アイコン
// =========================

	const float slotSize = 62.0f;
	const float slotGap = 8.0f;

	for (int i = 0; i < 4; ++i)
	{
		const float slotX =
			m_x + 8.0f +
			i * (slotSize + slotGap);

		const float slotY =
			m_y + 42.0f;

		const bool own =
			m_player->GetOwnJutsu(i);

		int srcX = 0;
		int srcY = 0;

		switch (i)
		{
		case 0: // 蛸
			if (own)
			{
				srcX = 750;
				srcY = 400;
			}
			else
			{
				srcX = 400;
				srcY = 400;
			}
			break;

		case 1: // 虎
			if (own)
			{
				srcX = 400;
				srcY = 750;
			}
			else
			{
				srcX = 50;
				srcY = 750;
			}
			break;

		case 2: // 蛙
			if (own)
			{
				srcX = 400;
				srcY = 50;
			}
			else
			{
				srcX = 50;
				srcY = 50;
			}
			break;

		case 3: // 鯱
			if (own)
			{
				srcX = 50;
				srcY = 400;
			}
			else
			{
				srcX = 750;
				srcY = 50;
			}
			break;
		}

		DrawRectExtendGraph(
			static_cast<int>(slotX),
			static_cast<int>(slotY),
			static_cast<int>(slotX + slotSize),
			static_cast<int>(slotY + slotSize),
			srcX,
			srcY,
			250,
			250,
			m_jutsuIconHandle,
			TRUE
		);
	}

	// =========================
// 影ゲージ
// =========================

	const int shadowGauge = m_player->GetShadowGauge();
	const int shadowGaugeMax = m_player->GetShadowGaugeMax();

	float shadowRate = 0.0f;

	if (shadowGaugeMax > 0)
	{
		shadowRate =
			static_cast<float>(shadowGauge) /
			static_cast<float>(shadowGaugeMax);
	}

	if (shadowRate < 0.0f)
	{
		shadowRate = 0.0f;
	}

	if (shadowRate > 1.0f)
	{
		shadowRate = 1.0f;
	}

	const int shadowBarX = panelX + 10;
	const int shadowBarY = panelY + 125;
	const int shadowBarWidth = 280;
	const int shadowBarHeight = 18;

	renderer->DrawTextL(
		Vector2d(
			m_x + 10.0f,
			m_y + 108.0f
		),
		"Shadow",
		Color(255, 255, 255),
		font,
		18,
		false
	);

	// ゲージ背景
	DrawBox(
		shadowBarX,
		shadowBarY,
		shadowBarX + shadowBarWidth,
		shadowBarY + shadowBarHeight,
		GetColor(60, 60, 60),
		TRUE
	);

	// ゲージ本体
	DrawBox(
		shadowBarX,
		shadowBarY,
		shadowBarX +
		static_cast<int>(shadowBarWidth * shadowRate),
		shadowBarY + shadowBarHeight,
		GetColor(180, 180, 180),
		TRUE
	);

	// ゲージ枠
	DrawBox(
		shadowBarX,
		shadowBarY,
		shadowBarX + shadowBarWidth,
		shadowBarY + shadowBarHeight,
		GetColor(255, 255, 255),
		FALSE
	);

	renderer->DrawTextL(
		Vector2d(
			m_x + 115.0f,
			m_y + 108.0f
		),
		std::to_string(shadowGauge)
		+ " / "
		+ std::to_string(shadowGaugeMax),
		Color(255, 255, 255),
		font,
		18,
		false
	);


	// =========================
	// クナイ回復ゲージ
	// =========================

	const int kunai = m_player->GetKunai();
	const float kunaiTimer = m_player->GetKunaiTimer();

	float kunaiRate = 0.0f;

	if (kunai >= 3)
	{
		kunaiRate = 1.0f;
	}
	else
	{
		kunaiRate = kunaiTimer / 20.0f;
	}

	if (kunaiRate < 0.0f)
	{
		kunaiRate = 0.0f;
	}

	if (kunaiRate > 1.0f)
	{
		kunaiRate = 1.0f;
	}

	const int kunaiBarX = panelX + 10;
	const int kunaiBarY = panelY + 172;
	const int kunaiBarWidth = 280;
	const int kunaiBarHeight = 18;

	renderer->DrawTextL(
		Vector2d(
			m_x + 10.0f,
			m_y + 155.0f
		),
		"Kunai",
		Color(255, 255, 255),
		font,
		18,
		false
	);

	// クナイ回復ゲージ背景
	DrawBox(
		kunaiBarX,
		kunaiBarY,
		kunaiBarX + kunaiBarWidth,
		kunaiBarY + kunaiBarHeight,
		GetColor(60, 60, 60),
		TRUE
	);

	// クナイ回復ゲージ本体
	DrawBox(
		kunaiBarX,
		kunaiBarY,
		kunaiBarX +
		static_cast<int>(kunaiBarWidth * kunaiRate),
		kunaiBarY + kunaiBarHeight,
		GetColor(180, 180, 180),
		TRUE
	);

	// クナイ回復ゲージ枠
	DrawBox(
		kunaiBarX,
		kunaiBarY,
		kunaiBarX + kunaiBarWidth,
		kunaiBarY + kunaiBarHeight,
		GetColor(255, 255, 255),
		FALSE
	);

	if (kunai >= 3)
	{
		renderer->DrawTextL(
			Vector2d(
				m_x + 110.0f,
				m_y + 155.0f
			),
			"MAX",
			Color(255, 255, 255),
			font,
			18,
			false
		);
	}
	else
	{
		const float remainTime = 20.0f - kunaiTimer;

		renderer->DrawTextL(
			Vector2d(
				m_x + 110.0f,
				m_y + 132.0f
			),
			std::to_string(
				static_cast<int>(remainTime)
			) + "s",
			Color(255, 255, 255),
			font,
			18,
			false
		);
	}

}