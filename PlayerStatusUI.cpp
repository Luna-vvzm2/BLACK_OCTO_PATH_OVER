#include "PlayerStatusUI.h"

#include "PlayerEntity.h"
#include "Game.h"
#include "Renderer.h"
#include <string>
#include <DxLib.h>

#include <cmath>

// クールタイムの赤い円形ゲージ
// 12時方向から時計回りに増加
static void DrawCooldownArc(
	int centerX,
	int centerY,
	int radius,
	float progress)
{
	if (progress <= 0.0f)
		return;

	if (progress > 1.0f)
		progress = 1.0f;

	constexpr float PI = 3.14159265358979323846f;
	constexpr int SEGMENTS = 72;

	const int color = GetColor(220, 25, 35);
	const int thickness = 6;

	for (int i = 0; i < SEGMENTS; ++i)
	{
		float startProgress =
			static_cast<float>(i) / SEGMENTS;

		if (startProgress >= progress)
			break;

		float endProgress =
			static_cast<float>(i + 1) / SEGMENTS;

		if (endProgress > progress)
			endProgress = progress;

		// -90度を開始位置にする（12時方向）
		float angle1 =
			-PI / 2.0f + startProgress * 2.0f * PI;

		float angle2 =
			-PI / 2.0f + endProgress * 2.0f * PI;

		int x1 = centerX +
			static_cast<int>(std::cos(angle1) * radius);

		int y1 = centerY +
			static_cast<int>(std::sin(angle1) * radius);

		int x2 = centerX +
			static_cast<int>(std::cos(angle2) * radius);

		int y2 = centerY +
			static_cast<int>(std::sin(angle2) * radius);

		DrawLine(
			x1, y1,
			x2, y2,
			color,
			thickness
		);
	}
}


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


    // ======================================
    // 回復アイテムUI（クナイ3本の右側）
    // ======================================

    // クナイ3本の表示幅
    // 48pxの画像を34px間隔で3本並べている
    const int kunaiWidth = 48 + 34 * 2;

    // クナイの右端から12px離す
    const int healX =
        static_cast<int>(m_x) + kunaiWidth + 12;

    // 回復アイテムのサイズ
    const int itemSize = 60;

    // クナイと縦方向の中心を揃える
    const int healY =
        static_cast<int>(m_y) - 6;

    // 回復アイテムの画像を描画
    if (m_healIconHandle >= 0)
    {
        DrawExtendGraph(
            healX,
            healY,
            healX + itemSize,
            healY + itemSize,
            m_healIconHandle,
            TRUE
        );
    }

    // 回復アイテムの所持数
    const std::string healCount =
        std::to_string(m_player->GetHealItem());

    const std::string& font =
        game->GatDebugFont();

    // 所持数の影
    renderer->DrawTextL(
        Vector2d(healX + 54.0f, healY + 13.0f),
        healCount,
        Color(20, 20, 20),
        font,
        38,
        false
    );

    // 所持数の文字
    renderer->DrawTextL(
        Vector2d(healX + 52.0f, healY + 11.0f),
        healCount,
        Color(255, 255, 255),
        font,
        38,
        false
    );

    const float slotSize = 72.0f;
    const float slotGap = 16.0f;

    for (int i = 0; i < 4; ++i)
    {
        const float slotX =
            m_x + i * (slotSize + slotGap);

        const float slotY =
            static_cast<float>(game->GetHeight())
            - slotSize - 18.0f;

        const bool own = m_player->GetOwnJutsu(i);

        // クールタイムの残り時間
        float interval = m_player->GetJutsuInterval(i);

        // 最大クールタイム
        float maxInterval =
            m_player->GetJutsuMaxInterval(i);

        // 文字入りアイコンを表示するか
        bool showText = false;

        if (i == 0)
        {
            // 蛸：使用中 または クールタイム中 は文字
            showText =
                m_player->GetIsJutsu(0) ||
                interval > 0.0f;
        }
        else
        {
            // 虎・蛙・鯱：クールタイム中は文字
            showText = interval > 0.0f;
        }

        // 未習得の場合は既存の表示を維持
        if (!own)
        {
            showText = (i != 0);
        }

        int srcX = 0;
        int srcY = 0;

        switch (i)
        {
        case 0: // 蛸
            srcX = showText ? 750 : 400;
            srcY = 400;
            break;

        case 1: // 虎
            srcX = showText ? 50 : 400;
            srcY = 750;
            break;

        case 2: // 蛙
            srcX = showText ? 50 : 400;
            srcY = 50;
            break;

        case 3: // 鯱
            if (showText)
            {
                srcX = 750;
                srcY = 50;
            }
            else
            {
                srcX = 50;
                srcY = 400;
            }
            break;
        }

        // ===========================
        // アイコンの描画
        // ===========================

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



        // ==========================================
        // 赤いクールタイムゲージ
        // ==========================================

     
        // 未習得の場合は表示しない
        if (!own)
            continue;

        // クールタイム中のみ表示
        if (interval <= 0.0f || maxInterval <= 0.0f)
            continue;

        // クールタイムの経過割合を計算
        float progress = 1.0f - (interval / maxInterval);

        // 0～1の範囲に制限
        if (progress < 0.0f)
            progress = 0.0f;

        if (progress > 1.0f)
            progress = 1.0f;

        // 開始直後でも見えるように最低4%表示
        if (progress < 0.04f)
            progress = 0.04f;

        // アイコンの中心座標
        const int centerX =
            static_cast<int>(slotX + slotSize / 2.0f);

        const int centerY =
            static_cast<int>(slotY + slotSize / 2.0f);

        // 赤い円形ゲージを描画
        DrawCooldownArc(
            centerX,
            centerY,
            35,
            progress
        );

    }

}
