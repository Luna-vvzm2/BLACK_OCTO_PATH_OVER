#include "AddEnemySpawner.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"
#include "EnemySpawner.h"
#include "KooniEnemyEntity.h"
#include "SpiritEnemyEntity.h"
#include "KasaTenguEnemyEntity.h"

#include <random>

namespace
{
    // ---- 出現位置(「画面内」は PlayerEntity::CollectEnemiesInScreen と同じ範囲の考え方) ----
    constexpr float SPAWN_RANGE_X = 600.0f;     // プレイヤーの左右この範囲に出現(画面の半幅700より少し内側)
    constexpr float SPAWN_MIN_DISTANCE_X = 250.0f;  // プレイヤーに近すぎる位置には出さない
    constexpr float SPAWN_MIN_GAP_X = 150.0f;   // 敵同士が重ならないための最小間隔
    constexpr float SPAWN_Y_OFFSET = -60.0f;    // プレイヤーより少し上から出して、重力で着地させる
    constexpr int   SPAWN_TRIES = 20;           // 位置の再抽選回数

    // 足場との重なり確認に使う、敵の大きさ(笠天狗の大きさで代表させる)
    constexpr float ENEMY_CHECK_WIDTH = 120.0f;
    constexpr float ENEMY_CHECK_HEIGHT = 180.0f;

    std::mt19937& Rng()
    {
        static std::mt19937 rng{ std::random_device{}() };
        return rng;
    }

    int RandomInt(int minValue, int maxValue)   // minValue 以上 maxValue 以下
    {
        std::uniform_int_distribution<int> dist(minValue, maxValue);
        return dist(Rng());
    }

    float RandomFloat(float minValue, float maxValue)
    {
        std::uniform_real_distribution<float> dist(minValue, maxValue);
        return dist(Rng());
    }
}

AddEnemySpawner::AddEnemySpawner(Scene* scene)
	: m_interval(10.0f)
	, m_scene(scene)
	, m_spawnPos{ 0.0f, 0.0f }
{
}

AddEnemySpawner::~AddEnemySpawner()
{
}

void AddEnemySpawner::Update(float deltaTime)
{
	static float time = 0.0f;
	time += deltaTime;

	if (time >= m_interval)
	{
		time = 0.0f;

		PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
        const PlayerEntity* player = playScene->GetPlayer();
        m_spawnPos = ChooseSpawnPos(player);

        int rand = RandomInt(0, 2);
        switch (rand)
        {
        case static_cast<int>(EnemyKind::Kooni):
            EnemySpawner::SpawnEnemy<KooniEnemyEntity>(playScene, m_spawnPos);
            break;

        case static_cast<int>(EnemyKind::Spirit):
            EnemySpawner::SpawnEnemy<SpiritEnemyEntity>(playScene, m_spawnPos);
            break;

        case static_cast<int>(EnemyKind::KasaTengu):
            EnemySpawner::SpawnEnemy<KasaTenguEnemyEntity>(playScene, m_spawnPos);
            break;

        default:
            break;
        }
	}
}

Vector2d AddEnemySpawner::ChooseSpawnPos(const PlayerEntity* player) const
{
    const Vector2d playerPos = player->GetPos();
    Vector2d candidate(playerPos.x, playerPos.y + SPAWN_Y_OFFSET);

    for (int i = 0; i < SPAWN_TRIES; ++i)
    {
        // プレイヤーの左右どちらかに、近すぎない距離で出す
        const float side = (RandomInt(0, 1) == 0) ? -1.0f : 1.0f;
        const float distance = RandomFloat(SPAWN_MIN_DISTANCE_X, SPAWN_RANGE_X);
        candidate = Vector2d(playerPos.x + side * distance, playerPos.y + SPAWN_Y_OFFSET);

        if (IsSpawnPosFree(candidate))
        {
            return candidate;
        }
    }

    // 空いている位置が見つからなければ、最後の候補をそのまま使う(めり込みは当たり判定の押し戻しに任せる)
    return candidate;
}

bool AddEnemySpawner::IsSpawnPosFree(const Vector2d& pos) const
{
    if (m_scene == nullptr) return true;

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor == nullptr || actor->GetType() != ActorType::Block) continue;

        BlockActor* block = static_cast<BlockActor*>(actor);

        // 通り抜けられるブロックは判定に含めない
        if (block->GetBlockType() == BlockType::Platform || block->GetBlockType() == BlockType::Hide) continue;

        CollisionComponent* col = block->GetCollision();
        if (col == nullptr) continue;

        const Vector2d blockPos = block->GetPos();
        const float halfW = (ENEMY_CHECK_WIDTH + col->GetWidth()) * 0.5f;
        const float halfH = (ENEMY_CHECK_HEIGHT + col->GetHeight()) * 0.5f;

        if (std::fabs(blockPos.x - pos.x) < halfW && std::fabs(blockPos.y - pos.y) < halfH)
        {
            return false;   // 壁・床にめり込む
        }
    }

    return true;
}