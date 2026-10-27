
#include "ShurikenUI.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "PlayScene.h"
#include "PlayerEntity.h"

#include <DxLib.h>

// =====================================
// クナイUIの設定
// =====================================

namespace
{
    constexpr int MAX_KUNAI = 3;

    constexpr int ICON_SIZE = 48;
    constexpr int ICON_SPACING = 34;

    // 回復の2秒前から演出開始
    constexpr float SPIN_START = 2.0f;

    // 回復の0.5秒前から黒色に戻る
    constexpr float FILL_START = 0.5f;

    constexpr float PI = 3.14159265358979323846f;
}

ShurikenUI::ShurikenUI(Scene* scene, float x, float y)
    : UIActor(scene)
{
    SetPosition(x, y);

    m_icon = AddComponent<SpriteComponent>(
        "assets/images/icon/kunai.png",
        false
    );
}

void ShurikenUI::SetCount(int count)
{
    if (count < 0) count = 0;
    if (count > MAX_KUNAI) count = MAX_KUNAI;

    m_count = count;
}

void ShurikenUI::Draw()
{
    if (!m_isVisible || !m_scene ||
        !m_icon || m_icon->GetHandle() < 0)
    {
        return;
    }

    const int handle = m_icon->GetHandle();

    int texW = 0;
    int texH = 0;

    GetGraphSize(handle, &texW, &texH);

    if (texW <= 0 || texH <= 0)
        return;

    // 既存のクールタイムを取得
    float remaining = 20.0f;

    PlayScene* playScene =
        dynamic_cast<PlayScene*>(m_scene);

    if (playScene && playScene->GetPlayer())
    {
        remaining =
            playScene->GetPlayer()->GetKunaiTimer();
    }

    const int startX = static_cast<int>(GetX());
    const int startY = static_cast<int>(GetY());

    for (int i = 0; i < MAX_KUNAI; ++i)
    {
        const int x = startX + i * ICON_SPACING;
        const int y = startY;

        const int centerX = x + ICON_SIZE / 2;
        const int centerY = y + ICON_SIZE / 2;

        const bool canUse = (i < m_count);

        // 次に回復するクナイだけアニメーション
        const bool recovering =
            (i == m_count && m_count < MAX_KUNAI);

        // 画像を黒色にする
        SetDrawBright(0, 0, 0);

        // =================================
        // ① 使用可能なクナイ
        // =================================
        if (canUse)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

            DrawExtendGraph(
                x, y,
                x + ICON_SIZE,
                y + ICON_SIZE,
                handle,
                TRUE
            );

            continue;
        }

        // =================================
        // ② 回復直前：回転アニメーション
        // =================================
        if (recovering &&
            remaining <= SPIN_START &&
            remaining > FILL_START)
        {
            // 0.0 → 1.0
            float t =
                (SPIN_START - remaining) /
                (SPIN_START - FILL_START);

            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            // 徐々に回転速度が遅くなる
            float slow = 1.0f - t;

            // 2回転して元の角度に戻る
            float angle =
                -26.0f * PI * slow * slow;

            // 使用不可なので薄く表示
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

            // 画像を中心で回転
            DrawRotaGraph(
                centerX,
                centerY,
                static_cast<double>(ICON_SIZE) / texW,
                angle,
                handle,
                TRUE
            );

            continue;
        }

        // =================================
        // ③ 使用不可：薄いクナイ
        // =================================
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 75);

        DrawExtendGraph(
            x, y,
            x + ICON_SIZE,
            y + ICON_SIZE,
            handle,
            TRUE
        );

        // =================================
        // ④ 回復直前：下から上に黒くする
        // =================================
        if (recovering &&
            remaining <= FILL_START &&
            remaining > 0.0f)
        {
            float fill =
                1.0f - remaining / FILL_START;

            if (fill < 0.0f) fill = 0.0f;
            if (fill > 1.0f) fill = 1.0f;

            // 下から上に向かって表示領域を広げる
            const int srcTop =
                static_cast<int>(texH * (1.0f - fill));

            const int destTop =
                y + static_cast<int>(
                    ICON_SIZE * (1.0f - fill)
                    );

            // 黒い部分を不透明で描画
            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                255
            );

            if (srcTop < texH)
            {
                DrawRectExtendGraph(
                    x,
                    destTop,
                    x + ICON_SIZE,
                    y + ICON_SIZE,
                    0,
                    srcTop,
                    texW,
                    texH - srcTop,
                    handle,
                    TRUE
                );
            }
        }
    }

    // 描画設定を元に戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetDrawBright(255, 255, 255);
}
