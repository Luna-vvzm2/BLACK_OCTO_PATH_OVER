#pragma once
#include "TreasureBox.h"

// 新聞を取得できる宝箱。
// プレイヤーの攻撃で開く(PlayerEntity::CheckAttackHit が TreasureBoxEntity::Open() を呼ぶ)と、
// 粉砕の演出のあとに新聞を入手して、PlayScene が新聞を画面に表示する(ゲームは止まる)。
// 通常の宝箱が落とす回復・コインは、新聞の宝箱では落とさない。
class NewsBoxEntity : public TreasureBoxEntity
{
public:
    NewsBoxEntity(Scene* scene, const Vector2d& pos, int newsId);

    bool Init() override;
    void Update(float deltaTime) override;
    void OnDead() override;

private:
    int m_newsId;           // 入手できる新聞のID(1~6)
    bool m_broken;          // 開いた(粉砕が始まった)か
    bool m_shown;           // 新聞の表示を依頼済みか
    float m_showTimer;      // 開いてからの経過時間
};
