#pragma once
#include "EntityActor.h"
#include "Vector2d.h"

#include <vector>

class CollisionComponent;
class PlayerEntity;
class PlayScene;

// 敵出現罠: プレイヤーが踏むと、プレイヤーの画面上に敵がランダムに出現し、罠は消える。
// 出現パターン(いずれか1つをランダムに選ぶ):
//   1. 雑魚敵 2~5体(ランダム)
//   2. 雑魚敵 2体 + エリート敵 1体
//   3. エリート敵 2体
// 雑魚敵 = 小鬼・怨霊、エリート敵 = 笠天狗。
class EnemySpawnTrap : public EntityActor {
public:
    EnemySpawnTrap(class Scene* scene, const Vector2d& pos, const Vector2d& size);
    ~EnemySpawnTrap() override = default;

    bool Init() override;
    void Update(float deltaTime) override;
    //void Draw() override;

    ActorType GetType() const override { return ActorType::Trap; }
    std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; }   // 仮: 素材が入るまで既存の罠と同じ

private:
    enum class EnemyKind { Kooni, Spirit, KasaTengu };
    enum class Phase { Waiting, Effect };

    struct SpawnPoint {
        EnemyKind kind;
        Vector2d pos;
    };

    void Trigger(PlayerEntity* player);                                   // 起動: パターンと出現位置を決める
    std::vector<EnemyKind> ChoosePattern() const;                         // 出現パターンをランダムに選ぶ
    Vector2d ChooseSpawnPos(const PlayerEntity* player) const;            // 画面内で、足場にめり込まない位置を選ぶ
    bool IsSpawnPosFree(const Vector2d& pos) const;                       // 壁・床と重ならないか
    void SpawnAll(PlayScene* playScene);                                  // 敵を実際に生成する

    Phase m_phase;
    float m_effectTimer;                    // 登場エフェクトの経過時間
    std::vector<SpawnPoint> m_spawnPoints;  // 出現させる敵と位置
};