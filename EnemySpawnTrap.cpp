#include "EnemySpawnTrap.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "BlockActor.h"
#include "CollisionComponent.h"
#include "EnemySpawner.h"
#include "KooniEnemyEntity.h"
#include "SpiritEnemyEntity.h"
#include "KasaTenguEnemyEntity.h"

#include "Game.h"
#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
    // ---- 出現パターン ----
    constexpr int NORMAL_MULTI_MIN = 2;           // パターン1: 雑魚敵の最少数
    constexpr int NORMAL_MULTI_MAX = 5;           // パターン1: 雑魚敵の最大数
    constexpr int NORMAL_WITH_ELITE_COUNT = 2;    // パターン2: 雑魚敵の数(要調整)
    constexpr int ELITE_ONLY_COUNT = 2;         // パターン3: エリート敵の数

    // ---- 出現位置(「画面内」は PlayerEntity::CollectEnemiesInScreen と同じ範囲の考え方) ----
    constexpr float SPAWN_RANGE_X = 600.0f;     // プレイヤーの左右この範囲に出現(画面の半幅700より少し内側)
    constexpr float SPAWN_MIN_DISTANCE_X = 250.0f;  // プレイヤーに近すぎる位置には出さない
    constexpr float SPAWN_MIN_GAP_X = 150.0f;   // 敵同士が重ならないための最小間隔
    constexpr float SPAWN_Y_OFFSET = -60.0f;    // プレイヤーより少し上から出して、重力で着地させる
    constexpr int   SPAWN_TRIES = 20;           // 位置の再抽選回数

    // 足場との重なり確認に使う、敵の大きさ(笠天狗の大きさで代表させる)
    constexpr float ENEMY_CHECK_WIDTH = 120.0f;
    constexpr float ENEMY_CHECK_HEIGHT = 180.0f;

    // ---- 登場エフェクト ----
    // 仮: 登場エフェクトが入るまでは、出現位置に広がる円を表示してから敵を出す。
    // エフェクトが実装されたら、エフェクト生成と SPAWN_EFFECT_TIME を差し替える。
    constexpr float SPAWN_EFFECT_TIME = 0.6f;
    constexpr float SPAWN_EFFECT_RADIUS = 90.0f;

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

EnemySpawnTrap::EnemySpawnTrap(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_phase(Phase::Waiting)
    , m_effectTimer(0.0f)
{
}

bool EnemySpawnTrap::Init() {
    if (!EntityActor::Init()) return false;

    if (m_collision) {
        m_collision->SetRect(m_initialSize.x, m_initialSize.y);
    }

    m_effect.Init(m_sprite, GetTransform());   // 起動前のエフェクト(ループ)を開始

    return true;
}

void EnemySpawnTrap::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    m_effect.Update(deltaTime);

    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    if (m_phase == Phase::Effect) {
        // 登場エフェクトが終わったら敵を出して、罠は消える
        m_effectTimer += deltaTime;
        if (m_effectTimer >= SPAWN_EFFECT_TIME) {
            SpawnAll(playScene);
            Destroy();
        }
        return;
    }

    // Phase::Waiting: プレイヤーが踏むのを待つ
    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    CollisionComponent* playerCol = player->GetCollision();
    if (!m_collision || !playerCol) return;

    if (m_collision->CheckCollision(playerCol)) {
        Trigger(player);
    }
}

void EnemySpawnTrap::Trigger(PlayerEntity* player)
{
    m_spawnPoints.clear();

    // 出現パターンを選び、敵ごとに出現位置を決める
    const std::vector<EnemyKind> pattern = ChoosePattern();
    for (EnemyKind kind : pattern)
    {
        SpawnPoint point;
        point.kind = kind;
        point.pos = ChooseSpawnPos(player);

        // 敵同士が重ならないよう、既に決めた位置から離れた位置を選び直す
        for (int i = 0; i < SPAWN_TRIES; ++i)
        {
            bool tooClose = false;
            for (const SpawnPoint& other : m_spawnPoints)
            {
                if (std::fabs(other.pos.x - point.pos.x) < SPAWN_MIN_GAP_X) { tooClose = true; break; }
            }
            if (!tooClose) break;
            point.pos = ChooseSpawnPos(player);
        }

        m_spawnPoints.push_back(point);
    }

    m_effect.PlayActivate();   // 罠の起動時のエフェクトを再生

    m_phase = Phase::Effect;
    m_effectTimer = 0.0f;
}

std::vector<EnemySpawnTrap::EnemyKind> EnemySpawnTrap::ChoosePattern() const
{
    // 雑魚敵(小鬼・怨霊)をランダムに1体選ぶ
    auto randomNormal = []() {
        return (RandomInt(0, 1) == 0) ? EnemyKind::Kooni : EnemyKind::Spirit;
        };

    std::vector<EnemyKind> result;

    switch (RandomInt(0, 2))   // 3パターンのいずれかを同じ確率で選ぶ
    {
    case 0:     // 雑魚敵 2～5体
    {
        const int count = RandomInt(NORMAL_MULTI_MIN, NORMAL_MULTI_MAX);
        for (int i = 0; i < count; ++i) result.push_back(randomNormal());
    } break;

    case 1:     // 雑魚敵 2体 + エリート敵 1体
        for (int i = 0; i < NORMAL_WITH_ELITE_COUNT; ++i) result.push_back(randomNormal());
        result.push_back(EnemyKind::KasaTengu);
        break;

    default:    // エリート敵 2体
        for (int i = 0; i < ELITE_ONLY_COUNT; ++i) result.push_back(EnemyKind::KasaTengu);
        break;
    }

    return result;
}

Vector2d EnemySpawnTrap::ChooseSpawnPos(const PlayerEntity* player) const
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

bool EnemySpawnTrap::IsSpawnPosFree(const Vector2d& pos) const
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

void EnemySpawnTrap::SpawnAll(PlayScene* playScene)
{
    for (const SpawnPoint& point : m_spawnPoints)
    {
        switch (point.kind)
        {
        case EnemyKind::Kooni:
            EnemySpawner::SpawnEnemy<KooniEnemyEntity>(playScene, point.pos);
            break;

        case EnemyKind::Spirit:
            EnemySpawner::SpawnEnemy<SpiritEnemyEntity>(playScene, point.pos);   
            break;

        case EnemyKind::KasaTengu:
            EnemySpawner::SpawnEnemy<KasaTenguEnemyEntity>(playScene, point.pos);
            break;

        }
    }

    m_spawnPoints.clear();
}