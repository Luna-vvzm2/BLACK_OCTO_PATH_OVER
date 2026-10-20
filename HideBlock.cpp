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
{
}

bool HideBlock::Init()
{
    return BlockActor::Init();
}

std::string HideBlock::GetTexturePath() const
{
    return HIDE_BLOCK_TEXTURE;
}