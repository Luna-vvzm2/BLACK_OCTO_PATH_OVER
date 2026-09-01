#pragma once
#include "BackGroundUI.h"
#include <string>

class HPComponent;
class TransformComponent;
class SpriteComponent;

class EnemyHPBar : public BackGroundUI
{
public:
    EnemyHPBar(Scene* scene, HPComponent* hp, const std::string& framePath = "assets/images/uies/HP_enemy_black.png");
    virtual ~EnemyHPBar() = default;

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    // スクリーン座標で位置を指定
    void SetPosition(float x, float y);

    void SetBarSize(float width, float height);

    void ShowFor(float seconds);
    void SetVisible(bool visible);

    // フレーム（枠）を相対でずらす（px）
    void SetFrameOffset(float offsetX, float offsetY);

    // transform が「左上座標」を表すか「中心座標」を表すか切替（デフォルト false = 左上）
    void SetPosIsCenter(bool center);

    // ゲージ幅/高さの比率（0..1）。例 0.9, 0.6
    void SetGaugeScale(float widthScale, float heightScale);

    // 内側パディング（左, top, right, bottom）px
    void SetPadding(float left, float top, float right, float bottom);

    void SetGaugeOffset(float offsetX, float offsetY);
    void SetMetsuValue(int value, int maxValue);
    void SetMetsuFullImagePath(const std::string& imagePath);
    void SetHPGaugeYOffset(float offsetY);
private:
    HPComponent* m_hp = nullptr;

    TransformComponent* m_transform = nullptr;
    SpriteComponent* m_frameSprite = nullptr;
    SpriteComponent* m_metsuFullSprite = nullptr;

    float m_maxWidth = 63.0f;
    float m_height = 8.0f;

    std::string m_barImagePath;
    std::string m_frameImagePath;
    std::string m_metsuFullImagePath = "assets/images/uies/metu_gage.png";

    bool m_visible = false;

    // 調整パラメータ（デフォルト値）
    float m_frameOffsetX = 0.0f;
    float m_frameOffsetY = 0.0f;
    bool  m_posIsCenter = false;

    float m_gaugeWidthScale = 0.92f;
    float m_gaugeHeightScale = 0.6f;

    float m_padLeft = 4.0f;
    float m_padTop = 2.0f;
    float m_padRight = 4.0f;
    float m_padBottom = 2.0f;
    float m_gaugeOffsetX = 0.0f;
    float m_gaugeOffsetY = 0.0f;

    int m_metsu = 0;
    int m_metsuMax = 100;
    float m_metsuOffsetY = 6.0f;
    float m_metsuHeight = 6.0f;
    float m_hpGaugeYOffset = 42.0f;
};
