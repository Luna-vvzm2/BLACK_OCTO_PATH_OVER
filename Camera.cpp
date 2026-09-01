#include "Camera.h"
#include <algorithm>

Camera::Camera(float screenWidth, float screenHeight)
    : m_screenWidth(screenWidth),
    m_screenHeight(screenHeight),
    m_zoom(1.0f),
    m_center(Vector2d::Zero()),
    m_boundsMin(Vector2d::Zero()),
    m_boundsMax(Vector2d::One()),
    m_tileHalfSize(Vector2d::Zero())
{
}

// カメラの中心をセット（クランプ付き）
void Camera::SetCenter(const Vector2d& center) {
    m_center = ClampCenter(center);
}

// クランプ処理：左端・右端・上端・下端をブロック端に合わせる
Vector2d Camera::ClampCenter(const Vector2d& center) const {
    float halfW = m_screenWidth / 2.0f / m_zoom;
    float halfH = m_screenHeight / 2.0f / m_zoom;

    Vector2d clamped;

    // 左上はそのまま
    clamped.x = std::max(center.x, m_boundsMin.x + halfW);
    clamped.y = std::max(center.y, m_boundsMin.y + halfH);

    // 右下はタイル端に合わせて補正
    clamped.x = std::min(clamped.x, m_boundsMax.x - halfW - m_tileHalfSize.x * 2.0f);
    clamped.y = std::min(clamped.y, m_boundsMax.y - halfH - m_tileHalfSize.y * 2.0f);

    return clamped;
}

// ワールド座標 → 画面座標
Vector2d Camera::WorldToScreen(const Vector2d& worldPos) const {
    Vector2d screenCenter(m_screenWidth / 2.0f, m_screenHeight / 2.0f);
    return (worldPos - m_center) * m_zoom + screenCenter;
}

void Camera::SetZoom(float zoom) {
    m_zoom = (zoom > 0.0f) ? zoom : 1.0f;
}

float Camera::GetZoom() const {
    return m_zoom;
}

void Camera::SetBounds(const Vector2d& min, const Vector2d& max) {
    m_boundsMin = min;
    m_boundsMax = max;
}

Vector2d Camera::GetCenter() const {
    return m_center;
}

void Camera::SetTileHalfSize(const Vector2d& halfSize) {
    m_tileHalfSize = halfSize;
}