#include "DamageTrap.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Renderer.h"

namespace
{
    constexpr int   DAMAGE_TRAP_DAMAGE = 15;
    constexpr float DAMAGE_TRAP_KNOCKBACK = 250.0f;        // 仮: 小鬼・笠天狗と同じ
    constexpr float DAMAGE_TRAP_EXPLOSION_TIME = 0.3f;     // 仮: 爆発演出の長さ
    constexpr float DAMAGE_TRAP_BLAST_RADIUS = 120.0f;     // 仮: 爆発演出の最大半径
}

DamageTrap::DamageTrap(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_damage(DAMAGE_TRAP_DAMAGE)
    , m_exploded(false)
    , m_explosionTimer(0.0f)
{
}

bool DamageTrap::Init() {
    if (!EntityActor::Init()) return false;

    if (m_collision) {
        constexpr float COLLISION_HEIGHT = 50.0f;   
        m_collision->SetRect(m_initialSize.x, COLLISION_HEIGHT);
        m_collision->SetOffset(Vector2d(0.0f, (m_initialSize.y - COLLISION_HEIGHT) * 0.5f));
    }

    m_effect.Init(m_sprite, GetTransform());   // 起動前のエフェクト(ループ)を開始

    return true;
}

void DamageTrap::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    m_effect.Update(deltaTime);

    // 爆発後は演出が終わるまで待って消える
    if (m_exploded) {
        m_explosionTimer += deltaTime;
        if (m_explosionTimer >= DAMAGE_TRAP_EXPLOSION_TIME) {
            Destroy();
        }
        return;
    }

    // PlayScene経由でプレイヤーを取得
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    CollisionComponent* playerCol = player->GetCollision();
    if (!m_collision || !playerCol) return;

    // 踏んだ(重なった)ら爆発
    if (m_collision->CheckCollision(playerCol)) {
        Explode(player);
    }
}

void DamageTrap::Explode(PlayerEntity* player)
{
    m_exploded = true;
    m_explosionTimer = 0.0f;
    m_effect.PlayActivate();   // 起動時のエフェクトを再生

    // 罠の中心から見て外向きにノックバック
    const float dir = (player->GetPos().x >= GetPos().x) ? 1.0f : -1.0f;
    player->TakeDamage(m_damage, Vector2d(dir * DAMAGE_TRAP_KNOCKBACK, 0.0f));
}