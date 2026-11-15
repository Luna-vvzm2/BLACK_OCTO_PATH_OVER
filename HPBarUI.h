#pragma once
#include "UIActor.h"
#include "Vector2d.h"

class HPComponent;
class TransformComponent;
class SpriteComponent;

class HPBarUI : public UIActor
{
public:
    HPBarUI(Scene* scene, HPComponent* hp);
    virtual ~HPBarUI();

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void SetBarSize(float width, float height);
    void SetPosition(float x, float y);

    float GetBarHeight() const { return m_height; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    bool IsVisible() const { return m_isVisible; }
private:
    HPComponent* m_hp = nullptr;

    TransformComponent* m_transform = nullptr;
    // SpriteComponent* m_sprite = nullptr;     // 赤ゲージ
    SpriteComponent* m_backBar = nullptr;    // フレーム

    float m_maxWidth;
    float m_height;
    // 影ゲージの波アニメーション用タイマー
    float m_shadowWaveTime = 0.0f;

    // HP・影ゲージ共通の背景画像
    int m_frameHandle = -1;

    std::string m_barImagePath;
    std::string m_frameImagePath;

    Vector2d m_barOffset = { 0, 0 }; // フレーム内のゲージ位置

    float m_displayRatio = 1.0f; // 画面表示用のHP割合（0.0 ～ 1.0）
    bool m_isVisible = true;
};
