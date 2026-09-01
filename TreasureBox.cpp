#include "TreasureBox.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "DropItemEntity.h"
#include "EnemyEntity.h"
#include "EntityActor.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "CollisionComponent.h"

TreasureBoxEntity::TreasureBoxEntity(Scene* scene,
    const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(64, 55))
    , m_isOpened(false)
    , m_isFading(false)
    , m_alpha(255.0f)
    , m_fadeSpeed(300.0f)
{
}

bool TreasureBoxEntity::Init()
{
    if (!EnemyEntity::Init())
        return false;

    m_hp = AddComponent<HPComponent>(1);
    m_dropTable.clear();

    // 必ずコインを落とす
    m_dropTable.push_back({ ItemType::Heal,1.0f });
    m_dropTable.push_back({ ItemType::Coin, 1.0f });
    return true;
}

void TreasureBoxEntity::Update(float deltaTime)
{
    // フェード中
    if (m_isFading)
    {
        m_fadeTimer += deltaTime;

        // 少し待ってからフェード開始
        if (m_fadeTimer >= m_fadeDelay)
        {
            // m_fadeTime秒で255→0になる
            m_alpha -= (255.0f / m_fadeTime) * deltaTime;

            if (m_alpha < 0.0f)
                m_alpha = 0.0f;

            if (m_sprite)
            {
                m_sprite->SetAlpha((int)m_alpha);
            }

            // 完全に消えたら削除
            if (m_alpha <= 0.0f)
            {
                SetState(Actor::State::Dead);
            }
        }

        return;
    }


    if (GetState() == Actor::State::Dead)
        return;


    if (m_isOpened)
    {
        OnDead();

        // フェード開始
        m_isFading = true;

        return;
    }
}

std::string TreasureBoxEntity::GetTexturePath() const
{
    return "Assets/images/objects/crateSmall.png";
}


void TreasureBoxEntity::OnDead()
{
    if (m_isFading)
        return;

    // ドロップだけ実行
    for (const auto& drop : m_dropTable)
    {
        float r = static_cast<float>(rand()) / RAND_MAX;

        if (r <= drop.probability)
        {
            SpawnItem(drop.type);
        }
    }

    // Deadにはしない
    m_isFading = true;
}