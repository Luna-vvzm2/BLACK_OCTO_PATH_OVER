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

PlayerStatusUI::~PlayerStatusUI()
{
	if (m_jutsuIconHandle >= 0) DeleteGraph(m_jutsuIconHandle);
	if (m_healIconHandle >= 0) DeleteGraph(m_healIconHandle);
}

bool PlayerStatusUI::Init()
{
	if (!UIActor::Init())
	{
		return false;
	}

	m_jutsuIconHandle =
		LoadGraph("assets/images/uies/ninjutu_icon_sheet.png");

	m_healIconHandle = LoadGraph("assets/images/icon/healItem.png");

	if (m_jutsuIconHandle == -1 || m_healIconHandle == -1)
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
    if (!m_player || !m_scene) return;
    Game* game = m_scene->GetGame();
    if (!game) return;
    Renderer* renderer = game->GetRenderer();
    if (!renderer) return;

    const int healX = static_cast<int>(m_x + 92.0f);
    const int healY = static_cast<int>(m_y);
    const int itemSize = 78;
    if (m_healIconHandle >= 0)
        DrawExtendGraph(healX, healY, healX + itemSize, healY + itemSize,
            m_healIconHandle, TRUE);

    const std::string healCount = std::to_string(m_player->GetHealItem());
    const std::string& font = game->GatDebugFont();
    renderer->DrawTextL(Vector2d(m_x + 156.0f, m_y + 15.0f),
        healCount, Color(20, 20, 20), font, 38, false);
    renderer->DrawTextL(Vector2d(m_x + 154.0f, m_y + 13.0f),
        healCount, Color(255, 255, 255), font, 38, false);
	const float slotSize = 72.0f;
	const float slotGap = 16.0f;

	for (int i = 0; i < 4; ++i)
	{
		const float slotX =
			m_x +
			i * (slotSize + slotGap);

		const float slotY =
			static_cast<float>(game->GetHeight()) - slotSize - 18.0f;

		const bool own =
			m_player->GetOwnJutsu(i);

		int srcX = 0;
		int srcY = 0;

		switch (i)
		{
		case 0: // ‘û
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

		case 1: // ŒÕ
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

		case 2: // Š^
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

		case 3: // éÍ
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

}
