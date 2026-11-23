#include "SpiritBulletEntity.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"

SpiritBulletEntity::SpiritBulletEntity(
    Scene* scene,
    const Vector2d& pos,
    const Vector2d& velocity,
    float deleteRange,
    const std::string& texturePath,
    int damage,
    const Vector2d& drawSize,
    bool rotate,
    float rotateInterval,
    float rotateStep
)
    : EnemyBullet(
        scene,
        pos,
        velocity,
        deleteRange,
        texturePath,
        damage,
        drawSize,
        rotate,
        rotateInterval,
        rotateStep
    )
    , m_faceLeft(velocity.x< 0.0f)
{
}

bool SpiritBulletEntity::Init()
{
    if (!EnemyBullet::Init())
    {
        return false;
    }

    // 怨霊の弾：15 × 15 px
    m_collision->SetRect(15.0f, 15.0f);

    // Reuse the sprite created by EnemyBullet::Init().

    if (!m_sprite->LoadTextureDiv(
        "assets/images/Enemy/onryoAttack/spritesheet.png",
        5,
        2))
    {
        return false;
    }
    TransformComponent* transform = GetComponent<TransformComponent>();

    if (transform != nullptr)
    {
        transform->SetScale(Vector2d(2.0f, 2.0f));
    }
    // アニメーションコンポーネント
    m_animation = AddComponent<AnimationComponent>();

    m_animation->SetSprite(m_sprite);

    // 待機アニメーション
    AnimationClip AttackClip;
    AttackClip.frames = { 0, 1, 2, 3, 4 ,5, 6, 7, 8, 9 };
    AttackClip.speed = 0.10f;
    AttackClip.loop = true;

    m_animation->AddClip("Attack", AttackClip);
    m_animation->Play("Attack");
    if (m_sprite != nullptr)
    {
        m_sprite->SetFlipX(m_faceLeft);
    }
    return true;
}