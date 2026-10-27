#include "HideBlock.h"

#include "SpriteComponent.h"

namespace
{
    const char* HIDE_BLOCK_TEXTURE = "assets/images/blocks/yagura3.png";

    constexpr float HIDE_BLOCK_OVERLAP_ALPHA = 0.7f;   // プレイヤーが重なっている間の不透明度
    constexpr float HIDE_BLOCK_FADE_SPEED = 5.0f;      // 1秒あたりの不透明度の変化量(大きいほど素早く変わる)
}

HideBlock::HideBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
    , m_playerOverlapped(false)
    , m_alpha(1.0f)
{
}

bool HideBlock::Init()
{
    return BlockActor::Init();
}

void HideBlock::Update(float deltaTime)
{
    BlockActor::Update(deltaTime);

    // 目標の不透明度に向けて、少しずつ近づける(フェード)
    const float target = m_playerOverlapped ? HIDE_BLOCK_OVERLAP_ALPHA : 1.0f;
    const float step = HIDE_BLOCK_FADE_SPEED * deltaTime;

    if (m_alpha < target)
    {
        m_alpha = (m_alpha + step < target) ? m_alpha + step : target;
    }
    else if (m_alpha > target)
    {
        m_alpha = (m_alpha - step > target) ? m_alpha - step : target;
    }

    // スプライトの不透明度に反映する(0~255)
    if (m_sprite != nullptr)
    {
        m_sprite->SetAlpha(static_cast<int>(255.0f * m_alpha));
    }
}

std::string HideBlock::GetTexturePath() const
{
    return HIDE_BLOCK_TEXTURE;
}