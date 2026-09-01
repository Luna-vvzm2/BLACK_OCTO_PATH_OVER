#pragma once
#include "Vector2d.h"

class Camera
{
public:
    Camera(float screenWidth, float screenHeight);

    // 中心座標
    void SetCenter(const Vector2d& center);
    Vector2d GetCenter() const;

    // ワールド座標 → 画面座標
    Vector2d WorldToScreen(const Vector2d& worldPos) const;

    // ズーム
    void SetZoom(float zoom);
    float GetZoom() const;

    // カメラ移動範囲
    void SetBounds(const Vector2d& min, const Vector2d& max);


    // マップのタイル半分サイズをセット
    void SetTileHalfSize(const Vector2d& halfSize);

private:
    float m_screenWidth;
    float m_screenHeight;
    float m_zoom;
    Vector2d m_center;

    Vector2d m_boundsMin;
    Vector2d m_boundsMax;


    Vector2d m_tileHalfSize{ 0, 0 };

    Vector2d ClampCenter(const Vector2d& center) const;
};