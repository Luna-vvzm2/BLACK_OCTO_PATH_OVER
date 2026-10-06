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

    // ---- 蛸の発動側(プレイヤー)から呼ぶ ----
    // プレイヤーに重なっている隠れブロックを隠れ状態にする。重なっていなければfalse(発動しても隠れない)。
    static bool TryHide(Scene* scene, const PlayerEntity* player);
    // 蛸の終了時に呼ぶ。シーン内の全隠れブロックの隠れ状態を解除する。
    static void ReleaseAll(Scene* scene);

    // ---- 敵側から呼ぶ ----
    // true のとき、そのプレイヤーは隠れている(索敵フラグ強制OFF)。
    static bool IsHidingPlayer(Scene* scene, const PlayerEntity* player);

    // ---- 個別操作・取得 ----
    bool IsHiding() const { return m_hiding; }
    bool IsOverlapping(const PlayerEntity* player) const;   // 当たり判定矩形が重なっているか
    void Hide(const PlayerEntity* player);
    void Release();

private:
    static HideBlock* FindOverlapping(Scene* scene, const PlayerEntity* player);

    std::string GetTexturePath() const override;

    bool m_hiding;
    const PlayerEntity* m_hidingPlayer;
};
