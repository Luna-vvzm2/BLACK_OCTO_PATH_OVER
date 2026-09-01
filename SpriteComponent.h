#pragma once
#include "Component.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "Vector2d.h"

class Renderer;

// -------------------------------------------
// SpriteComponent: 描画のみ担当
// アニメーションフレームを AnimationComponent から渡す
// -------------------------------------------
class SpriteComponent : public Component {
public:
    SpriteComponent(Actor* actor, bool useCamera = true); // 引数なしコンストラクタ
    SpriteComponent(Actor* actor, const std::string& texturePath, bool useCamera = true);
    ~SpriteComponent() override;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // 分割画像の読み込み
    bool LoadTextureDiv(const std::string& path, int xNum, int yNum);
    void SetFrame(int index);   // 指定フレームに切り替え
    void SetEffectFrames(const std::vector<int>& frames);
    void SetTexture(int handle); // 指定ハンドルに切り替え

    bool LoadTexture(const std::string& path);
    void SetSize(float w, float h);
    static void ReleaseTextures();

    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }

    int GetCurrentFrame() const { return m_currentFrame; }
    void SetDrawSize(float w, float h) { m_drawW = w; m_drawH = h; }
    void SetFlipH(bool flipH) { m_flipH = flipH; }

    int GetHandle() const { return m_handle; }
    void SetAlpha(int alpha) { m_alpha = alpha; }
    int GetAlpha() const { return m_alpha; }
    bool GetFlipH() const;
    void SetFlipV(bool flip)
    {
        m_flipV = flip;
    }

    void SetFlipX(bool flip) { m_flipX = flip; }
    bool GetFlipX() const { return m_flipX; }

    Vector2d GetDrawOffset() { return m_drawOffset; }
    void SetDrawOffset(float x, float y) { m_drawOffset = Vector2d{ x, y }; }
private:
    int m_handle;
    float m_width;
    float m_height;
    std::string m_texturePath;

    static std::unordered_map<std::string, int> s_textureCache;
    static std::unordered_map<std::string, std::vector<int>> s_textureDivCache;

    //画像位置調整
    Vector2d m_drawOffset = Vector2d::Zero();

    float m_drawW = 0;
    float m_drawH = 0;
    bool m_flipH = false;
    bool  m_flipV = false;

    bool m_flipX = false;

    std::vector<int> m_frames;   // 分割されたフレームのハンドル
    int m_currentFrame = 0;      // 現在のフレーム番号
    int m_alpha = 255;
    bool m_useCamera;
};