#pragma once

#include "BlockActor.h"

#include <string>

class PlayerEntity;
class Scene;

class HideBlock : public BlockActor
{
public:
    explicit HideBlock(
        Scene* scene,
        const Vector2d& pos = Vector2d::Zero(),
        const Vector2d& colSize = { 104, 104 },
        const Vector2d& texSize = { 104, 104 });
    ~HideBlock() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    BlockType GetBlockType() const override { return BlockType::Hide; }

    // ---- 敵側から呼ぶ ----
    // true のとき、そのプレイヤーはいずれかの隠れブロックで隠れている(索敵フラグ強制OFF)。
    static bool IsHidingPlayer(Scene* scene, const PlayerEntity* player);

    bool IsHiding() const { return m_hiding; }
    bool IsOverlapping(const PlayerEntity* player) const;   // 当たり判定矩形が重なっているか

private:
    PlayerEntity* FindPlayer() const;
    static bool IsHideNinjutsuActive(PlayerEntity* player);  // 蛸の発動中かどうか(判定はここ1か所)

    std::string GetTexturePath() const override;

    bool m_hiding;
    const PlayerEntity* m_hidingPlayer;
};