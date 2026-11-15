#pragma once

#include <vector>

class SpriteComponent;
class TransformComponent;

// 罠のエフェクト(スプライトシート方式)
//   起動前: loop_sheet.png を、コマごとの表示フレーム数どおりにループ再生する
//   起動時: on_sheet.png を、コマごとの表示フレーム数どおりに1回だけ再生する
// 罠自身の SpriteComponent を使って表示するので、罠側は Init / Update / 起動時の3か所から呼ぶだけでよい。
class TrapEffect {
public:
    TrapEffect();

    // 罠の Init から呼ぶ。起動前のエフェクト(ループ)を開始する。
    // 画像が読み込めなかった場合は false(罠は元のスプライトのまま動く)。
    bool Init(SpriteComponent* sprite, TransformComponent* transform);

    // 罠の Update から毎フレーム呼ぶ。
    void Update(float deltaTime);

    // 罠が起動した瞬間に呼ぶ。起動時のエフェクトを1回だけ再生する。
    void PlayActivate();

    bool IsActivating() const { return m_state == State::Activating; }        // 起動エフェクトの再生中
    bool IsActivateFinished() const { return m_state == State::Finished; }    // 起動エフェクトの再生が終わった

private:
    enum class State { Loop, Activating, Finished };

    bool StartSheet(const char* path, int cols, int rows, const std::vector<int>& frameCounts, bool loop, float offsetX, float offsetY);

    SpriteComponent* m_sprite;
    State m_state;
    std::vector<int> m_frameCounts;   // 再生中のシートの、コマごとの表示フレーム数(60fps基準)
    bool m_loop;
    int m_frameIndex;
    float m_timer;
};