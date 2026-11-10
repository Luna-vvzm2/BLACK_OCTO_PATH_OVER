#include "SmokeActor.h"
#include "PlayScene.h"
#include "Camera.h"

#include <DxLib.h>
#include <algorithm>

// 煙幕の持続時間
namespace
{
    constexpr float SMOKE_DURATION = 2.0f;
    constexpr float SMOKE_FADE_DURATION = 0.5f;
    constexpr float SMOKE_TOTAL_DURATION =
        SMOKE_DURATION + SMOKE_FADE_DURATION;

    // 煙幕の描画時の最大透明度
    constexpr int SMOKE_MAX_ALPHA = 150;
}

// コンストラクタ
SmokeActor::SmokeActor(
    Scene* scene,
    const Vector2d& position,
    const Vector2d& size
)
    : Actor(scene)
    , m_position(position)
    , m_size(size)
    , m_elapsedTime(0.0f)
    , m_expiredNotified(false)
{
}

// 更新処理
void SmokeActor::Update(float deltaTime)
{
    if (IsDead())
        return;

    Actor::Update(deltaTime);

    m_elapsedTime += deltaTime;

    // 持続時間が終了したら、煙幕終了処理を一度だけ呼ぶ
    if (m_elapsedTime >= SMOKE_TOTAL_DURATION)
    {
        if (!m_expiredNotified)
        {
            m_expiredNotified = true;

            // 敵が別の煙幕に入っているかなどの再判定は
            // PlayScene側に任せる
            PlayScene* playScene =
                dynamic_cast<PlayScene*>(m_scene);

            if (playScene != nullptr)
            {
                playScene->OnSmokeExpired(this);
            }
        }

        SetState(State::Dead);
    }
}

// 描画処理
void SmokeActor::Draw()
{
    if (IsDead() || !IsSmokeActive())
        return;

    PlayScene* playScene =
        dynamic_cast<PlayScene*>(m_scene);

    if (playScene == nullptr)
        return;

    // フェードアウト開始前は最大透明度を維持
    float fadeRate = 1.0f;

    if (m_elapsedTime > SMOKE_DURATION)
    {
        fadeRate =
            1.0f -
            (m_elapsedTime - SMOKE_DURATION)
            / SMOKE_FADE_DURATION;
    }

    fadeRate = std::clamp(fadeRate, 0.0f, 1.0f);

    const int alpha = static_cast<int>(
        SMOKE_MAX_ALPHA * fadeRate
        );

    if (alpha <= 0)
        return;

    // ワールド座標からスクリーン座標へ変換
    const Camera& camera = playScene->GetCamera();

    const Vector2d topLeft = camera.WorldToScreen(
        Vector2d(
            m_position.x - m_size.x * 0.5f,
            m_position.y - m_size.y * 0.5f
        )
    );

    const Vector2d screenCenter =
        camera.WorldToScreen(m_position);

    const float halfWidth = m_size.x * 0.5f;
    const float halfHeight = m_size.y * 0.5f;

    // 煙を複数の楕円で表現する
    // 煙幕の範囲判定には影響しない描画用の処理
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    const int smokeColor = GetColor(35, 35, 45);

    const int centerX = static_cast<int>(screenCenter.x);
    const int centerY = static_cast<int>(screenCenter.y);

    const int width = static_cast<int>(halfWidth);
    const int height = static_cast<int>(halfHeight);

    // 中央
    DrawOval(
        centerX - width,
        centerY - height,
        centerX + width,
        centerY + height,
        smokeColor,
        TRUE
    );

    // 左側
    DrawOval(
        centerX - width,
        centerY - height / 2,
        centerX,
        centerY + height / 2,
        smokeColor,
        TRUE
    );

    // 右側
    DrawOval(
        centerX,
        centerY - height / 2,
        centerX + width,
        centerY + height / 2,
        smokeColor,
        TRUE
    );

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 指定座標が煙幕の範囲内にあるか
bool SmokeActor::Contains(const Vector2d& position) const
{
    if (!IsSmokeActive())
        return false;

    const float halfWidth = m_size.x * 0.5f;
    const float halfHeight = m_size.y * 0.5f;

    return
        position.x >= m_position.x - halfWidth &&
        position.x <= m_position.x + halfWidth &&
        position.y >= m_position.y - halfHeight &&
        position.y <= m_position.y + halfHeight;
}

// 煙幕が有効か
bool SmokeActor::IsSmokeActive() const
{
    return
        !IsDead() &&
        m_elapsedTime < SMOKE_TOTAL_DURATION;
}

// 煙幕の中心座標
Vector2d SmokeActor::GetPosition() const
{
    return m_position;
}

// 煙幕の範囲
Vector2d SmokeActor::GetSize() const
{
    return m_size;
}