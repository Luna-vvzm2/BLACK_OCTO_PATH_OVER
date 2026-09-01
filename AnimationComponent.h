#pragma once
#include "Component.h"
#include <unordered_map>
#include <vector>
#include <string>

class SpriteComponent;

// -------------------------------------------
// AnimationClip: フレーム配列 + 再生速度 + ループ設定
// -------------------------------------------
struct AnimationClip {
    std::vector<int> frames;
    std::vector<float> frameDurations;
    float speed = 0.1f;
    bool loop = true;
};

// -------------------------------------------
// AnimationComponent: アニメ管理
// SpriteComponent にフレームを反映
// -------------------------------------------
class AnimationComponent : public Component {
public:
    explicit AnimationComponent(Actor* owner);
    ~AnimationComponent() override {}

    void Update(float dt) override;

    void AddClip(const std::string& name, const AnimationClip& clip);
    void Play(const std::string& name, bool reset = false);
    bool IsFinished() const;

    const std::string& GetCurrentName() const { return m_currentName; }

    void SetSprite(SpriteComponent* sprite) { m_sprite = sprite; }
    SpriteComponent* GetSprite() const { return m_sprite; }

private:
    SpriteComponent* m_sprite;
    std::unordered_map<std::string, AnimationClip> m_clips;

    AnimationClip* m_currentClip;
    std::string m_currentName;

    float m_timer;
    int m_frameIndex;
};