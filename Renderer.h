#pragma once
#include <DxLib.h>
#include "Vector2d.h"
#include "Color.h"
#include "Camera.h"
#include <string>
#include <unordered_map>
#include <vector>

struct NumberInfo {
    float value;
    int precision;
};

class Renderer
{
public:
    Renderer(HWND hwnd, int screenWidth, int screenHeight);
    ~Renderer();

    void Begin();
    void End();

    // --- 描画関数 ---
    void DrawLine(const Vector2d& p1, const Vector2d& p2, const Color& color, bool useCamera = true);
    void DrawCircle(const Vector2d& center, float radius, const Color& color, bool fill = false, bool useCamera = true);
    void DrawRect(const Vector2d& pos, float width, float height, const Color& color, bool fill = false, bool useCamera = true);
    void DrawRectCenter(const Vector2d& center, float width, float height, const Color& color, bool fill = false, bool useCamera = true);
    void DrawTextL(const Vector2d& pos, const std::string& text, const Color& color,
        const std::string& fontName = "メイリオ", int size = 24, bool useCamera = true);
    void DrawTextR(const Vector2d& pos, const std::string& text, const Color& color,
        const std::string& fontName = "メイリオ", int size = 24, bool useCamera = true);
    void DrawTextC(const Vector2d& pos, const std::string& text, const Color& color,
        const std::string& fontName = "メイリオ", int size = 24, bool useCamera = true);

    void DrawNumberFormatW(const Vector2d& pos, const Color& color, const std::string& fontName, int size,
        const std::string& format, const std::vector<NumberInfo>& numbers, bool useCamera = true);

    void DrawSprite(const Vector2d& pos, float scale, float angle, int handle, bool trans = true, bool useCamera = true);
    void DrawSpriteEx(const Vector2d& pos, float scaleX, float scaleY, float angle, int handle, bool trans = true,
        const Vector2d& center = Vector2d::Zero(), int alpha = 255, bool flipH = false, bool flipV = false, bool useCamera = true);

    // Camera 設定
    void SetCamera(const Camera* camera) { m_camera = camera; }
    float GetCameraZoom() const { return m_camera ? m_camera->GetZoom() : 1.0f; }
    void DrawBackground(const Vector2d& pos, int handle, bool useCamera);

    void DrawFullScreenFill(const Color& color, int alpha);
private:
    Vector2d ApplyCamera(const Vector2d& pos, bool useCamera) const;

    HWND m_window;
    int m_screenWidth;
    int m_screenHeight;
    int m_fontHandle;
    std::string m_currentFontName;
    int m_currentFontSize;
    std::unordered_map<std::string, int> m_fontCache;

    const Camera* m_camera = nullptr;
    int GetFontHandle(const std::string& fontName, int size);

};