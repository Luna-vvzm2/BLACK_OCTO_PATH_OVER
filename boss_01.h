#pragma once

#include "BossEntity.h"
#include <functional>
#include <vector>

// 洲条櫻：ステータス、待機、後退、やられのみを担当する。
// 既存ファイルへの登録・生成処理の追加は、このファイルでは行わない。
// 接続時は Scene::AddActor(new Boss_01(scene, position)) を使用する。
// 描画素材は Init 後に SetAnimationFrames で設定する（未設定時は描画なし）。
// シーン側は IsBattleFinished() または OnBattleFinished で戦闘終了を受け取る。
class Boss_01 : public BossEntity
{
public:
    enum class Behavior { Idle, Move, Defeated };

    struct Parameters
    {
        int maxHP = 2000;
        float defeatHPRatio = 0.30f;
        Vector2d hitboxSize = { 100.0f, 150.0f };
        float moveSpeed = 500.0f;
        // 距離は仕様書に数値がないため仮設定。開始と停止を分けて振動を防ぐ。
        float retreatStartDistance = 300.0f;
        float retreatStopDistance = 500.0f;
    };

    explicit Boss_01(Scene* scene, const Vector2d& pos = Vector2d::Zero());
    Boss_01(Scene* scene, const Vector2d& pos, const Parameters& parameters);

    bool Init() override;
    void Update(float deltaTime) override;
    int GetMaxHP() const override { return m_parameters.maxHP; }
    void TakeDamage(int damage, const Vector2d& knockback) override;
    void TakeMetsu(int metsu) override;
    void OnDead() override;

    Behavior GetBehavior() const { return m_behavior; }
    bool IsBattleFinished() const { return m_behavior == Behavior::Defeated; }
    int GetDefeatHP() const;
    const Parameters& GetParameters() const { return m_parameters; }

    // DxLibで読み込み済みのハンドルを渡す。所有権は呼び出し側に残る。
    // 待機3枚、後退6枚、片膝をついた呼吸ループ7枚を、それぞれ再生順で指定。
    // 使用中の画像はボスを破棄するまで解放しない。右向き原画が既定。
    bool SetAnimationFrames(const std::vector<int>& idle,
        const std::vector<int>& move, const std::vector<int>& defeated,
        bool sourceFacesRight = true);

    // 戦闘終了への遷移時に一度だけ通知。未設定でも終了状態は保持する。
    // コールバック内でこのActorを即時deleteせず、シーン変更は予約すること。
    std::function<void()> OnBattleFinished;

protected:
    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float deltaTime) override; // 今回は攻撃を実装しない。

private:
    void ChangeBehavior(Behavior behavior);
    void EnterDefeated();
    void PlayBehaviorAnimation();

    Parameters m_parameters;
    Behavior m_behavior = Behavior::Idle;
    bool m_initialized = false;
    bool m_sourceFacesRight = true;
};
