#include "HideBlock.h"

#include "Actor.h"
#include "CollisionComponent.h"
#include "PlayerEntity.h"
#include "Scene.h"

#include <cmath>

namespace
{
    // 仮: 隠れブロックの画像
    const char* HIDE_BLOCK_TEXTURE = "assets/images/Block/hideBlock.png";
}

HideBlock::HideBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
    , m_hiding(false)
    , m_hidingPlayer(nullptr)
{
}

bool HideBlock::Init()
{
    return BlockActor::Init();
}

void HideBlock::Update(float deltaTime)
{
    BlockActor::Update(deltaTime);

    // 隠れている間にプレイヤーがブロックから外れたら解除する
    if (m_hiding)
    {
        if (m_hidingPlayer == nullptr || m_hidingPlayer->IsDead() || !IsOverlapping(m_hidingPlayer))
        {
            Release();
        }
    }
}

bool HideBlock::IsOverlapping(const PlayerEntity* player) const
{
    if (player == nullptr || player->GetCollision() == nullptr || m_collision == nullptr)
    {
        return false;
    }

    // GetPos() を中心としたAABBの重なりで判定する
    const Vector2d difference = player->GetPos() - GetPos();
    const float combinedHalfWidth =
        m_collision->GetWidth() * 0.5f + player->GetCollision()->GetWidth() * 0.5f;
    const float combinedHalfHeight =
        m_collision->GetHeight() * 0.5f + player->GetCollision()->GetHeight() * 0.5f;

    return std::fabs(difference.x) <= combinedHalfWidth &&
        std::fabs(difference.y) <= combinedHalfHeight;
}

void HideBlock::Hide(const PlayerEntity* player)
{
    m_hiding = true;
    m_hidingPlayer = player;
}

void HideBlock::Release()
{
    m_hiding = false;
    m_hidingPlayer = nullptr;
}

HideBlock* HideBlock::FindOverlapping(Scene* scene, const PlayerEntity* player)
{
    if (scene == nullptr)
    {
        return nullptr;
    }

    for (Actor* actor : scene->GetActors())
    {
        if (actor == nullptr || actor->IsDead() || actor->GetType() != ActorType::Block)
        {
            continue;
        }

        BlockActor* block = static_cast<BlockActor*>(actor);
        if (block->GetBlockType() != BlockType::Hide)
        {
            continue;
        }

        HideBlock* hideBlock = static_cast<HideBlock*>(block);
        if (hideBlock->IsOverlapping(player))
        {
            return hideBlock;
        }
    }

    return nullptr;
}

bool HideBlock::TryHide(Scene* scene, const PlayerEntity* player)
{
    HideBlock* block = FindOverlapping(scene, player);
    if (block == nullptr)
    {
        return false;
    }

    block->Hide(player);
    return true;
}

void HideBlock::ReleaseAll(Scene* scene)
{
    if (scene == nullptr)
    {
        return;
    }

    for (Actor* actor : scene->GetActors())
    {
        if (actor == nullptr || actor->GetType() != ActorType::Block)
        {
            continue;
        }

        BlockActor* block = static_cast<BlockActor*>(actor);
        if (block->GetBlockType() == BlockType::Hide)
        {
            static_cast<HideBlock*>(block)->Release();
        }
    }
}

bool HideBlock::IsHidingPlayer(Scene* scene, const PlayerEntity* player)
{
    if (scene == nullptr || player == nullptr)
    {
        return false;
    }

    for (Actor* actor : scene->GetActors())
    {
        if (actor == nullptr || actor->IsDead() || actor->GetType() != ActorType::Block)
        {
            continue;
        }

        BlockActor* block = static_cast<BlockActor*>(actor);
        if (block->GetBlockType() != BlockType::Hide)
        {
            continue;
        }

        const HideBlock* hideBlock = static_cast<const HideBlock*>(block);
        if (hideBlock->m_hiding && hideBlock->m_hidingPlayer == player && hideBlock->IsOverlapping(player))
        {
            return true;
        }
    }

    return false;
}

std::string HideBlock::GetTexturePath() const
{
    return HIDE_BLOCK_TEXTURE;
}