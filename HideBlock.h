#pragma once

#include "BlockActor.h"

#include <string>

// 隠れる障害物。
// 重なり判定と隠れ状態の通知は EntityActor::MoveAndCollide で行い、
// ブロックはその結果(SetPlayerOverlapped)を受け取って、スプライトの不透明度(SetAlpha)をフェードさせるだけを行う。
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

    // EntityActor::MoveAndCollide から、プレイヤーとの重なりを毎フレーム通知される
    void SetPlayerOverlapped(bool overlapped) { m_playerOverlapped = overlapped; }
    bool IsPlayerOverlapped() const { return m_playerOverlapped; }

private:
    std::string GetTexturePath() const override;

    bool m_playerOverlapped;   // プレイヤーが重なっているか(EntityActor側から設定される)
    float m_alpha;             // 現在の不透明度(0.0~1.0)
};