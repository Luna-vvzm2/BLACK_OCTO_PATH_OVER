#include "Game.h"
#include "PlayScene.h"
#include "HPComponent.h"
#include "EnemySpawner.h"
#include "ArmorEnemyEntity.h"
#include "EnemyHPBar.h"
#include <cstdio>
#include "PlayerEntity.h"
class TestScene : public PlayScene {
public:
    using PlayScene::PlayScene;
    using Scene::RemoveDeadActors;
    void FillCapacity() {
        struct Filler : Actor {
            using Actor::Actor;
            ActorType GetType() const override { return ActorType::Block; }
        };
        while (m_actors.size() < m_actors.capacity()) AddActor(new Filler(this));
    }
};
int main() {
    DxLib_Init();
    Game game;
    TestScene scene(&game);
    auto* player = new PlayerEntity(&scene, Vector2d(0,0));
    scene.AddActor(player);
    for (int round = 0; round < 8; ++round) {
        auto* enemy = EnemySpawner::SpawnEnemy<ArmorEnemyEntity>(&scene, Vector2d(0,0));
        scene.FillCapacity();
        for (int hit = 0; hit < 100 && !enemy->IsDead(); ++hit) {
            player->CheckAttackHit({Vector2d(0,0), 300, 300, 100, 0});
        }
        std::printf("REGRESSION: deleting dead enemy and HP bar %d\n", round);
        scene.RemoveDeadActors();
    }
    EM_ASM({document.getElementById('status').textContent='REGRESSION PASS';});
    return 0;
}
