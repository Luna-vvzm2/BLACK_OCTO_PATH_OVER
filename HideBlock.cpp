#include "HideBlock.h"

#include "Actor.h"
#include "CollisionComponent.h"
#include "PlayerEntity.h"
#include "Scene.h"

namespace
{
    // 仮: 隠れブロックの画像。素材が入るまでは既存ブロックの画像パスに差し替えて確認する。
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

    // 重なっていて、かつ蛸の発動中のときだけ隠れ状態(外れた・蛸が終わったら自動で解除)
    PlayerEntity* player = FindPlayer();
    const bool hiding = player != nullptr
        && IsHideNinjutsuActive(player)
        && IsOverlapping(player);

    m_hiding = hiding;
    m_hidingPlayer = hiding ? player : nullptr;
}

PlayerEntity* HideBlock::FindPlayer() const
{
    if (m_scene == nullptr)
    {
        return nullptr;
    }

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor != nullptr && actor->GetType() == ActorType::Player && !actor->IsDead())
        {
            return static_cast<PlayerEntity*>(actor);
        }
    }

    return nullptr;
}

bool HideBlock::IsHideNinjutsuActive(PlayerEntity* player)
{
    // 蛸の発動中フラグ(違っていたら要変更)
    return player->GetIsOcto();
}

bool HideBlock::IsOverlapping(const PlayerEntity* player) const
{
    if (player == nullptr || player->GetCollision() == nullptr || m_collision == nullptr)
    {
        return false;
    }

    // CollisionComponent の矩形判定(オフセット込み)で重なりを見る
    return m_collision->CheckCollision(player->GetCollision());
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
        if (hideBlock->m_hiding && hideBlock->m_hidingPlayer == player)
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