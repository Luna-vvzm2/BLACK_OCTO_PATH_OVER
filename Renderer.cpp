#include "Renderer.h"
#include <sstream>
#include <iomanip>

Renderer::Renderer(HWND hwnd, int screenWidth, int screenHeight)
    : m_window(hwnd), m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_fontHandle(-1), m_currentFontSize(0)
{
}

Renderer::~Renderer() {
    for (auto& pair : m_fontCache) DeleteFontToHandle(pair.second);
    m_fontCache.clear();
}

void Renderer::Begin() {}
void Renderer::End() {}

Vector2d Renderer::ApplyCamera(const Vector2d& pos, bool useCamera) const {
    if (!useCamera || !m_camera) return pos;
    return m_camera->WorldToScreen(pos);
}

// --- 描画関数 ---
void Renderer::DrawLine(const Vector2d& p1, const Vector2d& p2, const Color& color, bool useCamera) {
    Vector2d c1 = ApplyCamera(p1, useCamera);
    Vector2d c2 = ApplyCamera(p2, useCamera);
    DrawLineAA(c1.x, c1.y, c2.x, c2.y, color.ToDxColor());
}

void Renderer::DrawCircle(const Vector2d& center, float radius, const Color& color, bool fill, bool useCamera) {
    Vector2d c = ApplyCamera(center, useCamera);
    float zoom = m_camera ? m_camera->GetZoom() : 1.0f;
    DrawCircleAA(c.x, c.y, radius * (useCamera ? zoom : 1.0f), 32, color.ToDxColor(), fill ? TRUE : FALSE);
}

void Renderer::DrawRect(const Vector2d& pos, float width, float height, const Color& color, bool fill, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    float zoom = m_camera ? m_camera->GetZoom() : 1.0f;
    DrawBoxAA(p.x, p.y, p.x + width * (useCamera ? zoom : 1.0f), p.y + height * (useCamera ? zoom : 1.0f), color.ToDxColor(), fill ? TRUE : FALSE);
}

void Renderer::DrawRectCenter(const Vector2d& center, float width, float height, const Color& color, bool fill, bool useCamera) {
    Vector2d c = ApplyCamera(center, useCamera);
    float zoom = m_camera ? m_camera->GetZoom() : 1.0f;
    float w = width * (useCamera ? zoom : 1.0f);
    float h = height * (useCamera ? zoom : 1.0f);
    DrawBoxAA(c.x - w / 2, c.y - h / 2, c.x + w / 2, c.y + h / 2, color.ToDxColor(), fill ? TRUE : FALSE);
}

void Renderer::DrawTextL(const Vector2d& pos, const std::string& text, const Color& color,
    const std::string& fontName, int size, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    int handle = GetFontHandle(fontName, size);
    DrawStringToHandle(static_cast<int>(p.x), static_cast<int>(p.y), text.c_str(), color.ToDxColor(), handle);
}

void Renderer::DrawTextR(const Vector2d& pos, const std::string& text, const Color& color,
    const std::string& fontName, int size, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    int handle = GetFontHandle(fontName, size);
    int w = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle);
    DrawStringToHandle(static_cast<int>(p.x - w), static_cast<int>(p.y), text.c_str(), color.ToDxColor(), handle);
}

void Renderer::DrawTextC(const Vector2d& pos, const std::string& text, const Color& color,
    const std::string& fontName, int size, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    int handle = GetFontHandle(fontName, size);
    int w = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle);
    DrawStringToHandle(static_cast<int>(p.x - w / 2), static_cast<int>(p.y), text.c_str(), color.ToDxColor(), handle);
}

void Renderer::DrawNumberFormatW(const Vector2d& pos, const Color& color,
    const std::string& fontName, int size,
    const std::string& format,
    const std::vector<NumberInfo>& numbers, bool useCamera) {
    std::string tmp = format;
    std::stringstream wss;

    for (size_t i = 0; i < numbers.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t idx = tmp.find(placeholder);
        if (idx != std::string::npos) {
            wss.str("");
            wss << std::fixed << std::setprecision(numbers[i].precision) << numbers[i].value;
            while ((idx = tmp.find(placeholder)) != std::string::npos) {
                tmp.replace(idx, placeholder.length(), wss.str());
            }
        }
    }

    Vector2d p = ApplyCamera(pos, useCamera);
    int handle = GetFontHandle(fontName, size);
    DrawStringToHandle(static_cast<int>(p.x), static_cast<int>(p.y), tmp.c_str(), color.ToDxColor(), handle);
}

void Renderer::DrawSprite(const Vector2d& pos, float scale, float angle, int handle, bool trans, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    float zoom = m_camera ? m_camera->GetZoom() : 1.0f;
    DrawRotaGraphF(p.x, p.y, scale * (useCamera ? zoom : 1.0f), angle, handle, trans ? TRUE : FALSE);
}

void Renderer::DrawSpriteEx(const Vector2d& pos, float scaleX, float scaleY, float angle, int handle, bool trans,
    const Vector2d& center, int alpha, bool flipH, bool flipV, bool useCamera) {
    Vector2d p = ApplyCamera(pos, useCamera);
    float zoom = m_camera ? m_camera->GetZoom() : 1.0f;
    float sx = scaleX * (useCamera ? zoom : 1.0f);
    float sy = scaleY * (useCamera ? zoom : 1.0f);
    if (flipH) sx = -sx;
    if (flipV) sy = -sy;

    int oldBlend, oldAlpha;
    GetDrawBlendMode(&oldBlend, &oldAlpha);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    DrawRotaGraph3F(p.x, p.y, center.x, center.y, sx, sy, angle, handle, trans ? TRUE : FALSE);

    SetDrawBlendMode(oldBlend, oldAlpha);
}

int Renderer::GetFontHandle(const std::string& fontName, int size) {
    std::string key = fontName + "_" + std::to_string(size);
    auto it = m_fontCache.find(key);
    if (it != m_fontCache.end()) return it->second;

    int handle = CreateFontToHandle(fontName.c_str(), size, 3, DX_FONTTYPE_NORMAL);
    if (handle == -1) MessageBoxA(m_window, "フォント生成失敗", "エラー", MB_OK);
    else m_fontCache[key] = handle;
    return handle;
}

void Renderer::DrawBackground(
    const Vector2d& pos,
    int handle,
    bool useCamera
)
{
    Vector2d p = ApplyCamera(pos, useCamera);

    DrawGraph(
        (int)p.x,
        (int)p.y,
        handle,
        TRUE
    );
}

void Renderer::DrawFullScreenFill(const Color& color, int alpha) {
    if (alpha <= 0) return;
    if (alpha > 255) alpha = 255;

    int oldBlend, oldAlpha;
    GetDrawBlendMode(&oldBlend, &oldAlpha);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // useCamera=false 相当。スクリーン座標系で全画面を塗る
    DrawBox(0, 0, m_screenWidth, m_screenHeight,
        color.ToDxColor(), TRUE);

    SetDrawBlendMode(oldBlend, oldAlpha);
}
