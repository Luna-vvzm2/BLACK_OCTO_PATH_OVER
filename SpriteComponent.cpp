#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "CollisionComponent.h"
#include "TextureManager.h"
#include "Actor.h"
#include "Scene.h"
#include "Game.h"
#include "Renderer.h"
#include <DxLib.h>
#include <iostream>
#include <unordered_set>

std::unordered_map<std::string, int>
SpriteComponent::s_textureCache;

std::unordered_map<std::string, std::vector<int>>
SpriteComponent::s_textureDivCache;

// --------------------
// コンストラクタ
// --------------------
SpriteComponent::SpriteComponent(Actor* actor, bool useCamera)
    : Component(actor)
    , m_handle(-1)
    , m_width(0.0f)
    , m_height(0.0f)
    , m_useCamera(useCamera)
{
}

SpriteComponent::SpriteComponent(Actor* actor, const std::string& texturePath, bool useCamera)
    : Component(actor)
    , m_handle(-1)
    , m_width(0.0f)
    , m_height(0.0f)
    , m_texturePath(texturePath)
    , m_useCamera(useCamera)
{
}
// --------------------
// デストラクタ
// --------------------
SpriteComponent::~SpriteComponent() {
}

// --------------------
// 初期化
// --------------------
bool SpriteComponent::Init() {
    if (!m_texturePath.empty())
        return LoadTexture(m_texturePath);
    return true;
}

// --------------------
// 更新（特になし）
// --------------------
void SpriteComponent::Update(float deltaTime) {}

// --------------------
// 描画
// --------------------
void SpriteComponent::Draw() {
    
    if (m_handle == -1) {
        /*DrawBox(
            -16,
            -16,
            16,
            16,
            GetColor(255, 0, 255),
            TRUE
        );*/

        return;
    }
    if (m_handle < 0)
    {
        std::cout
            << "invalid handle="
            << m_handle
            << std::endl;

        return;
    }
    auto transform = m_owner->GetComponent<TransformComponent>();
    if (!transform) return;

    Renderer* renderer = m_owner->GetScene()->GetGame()->GetRenderer();
    if (!renderer) return;

    int texW = 0, texH = 0;
    GetGraphSize(m_handle, &texW, &texH);

    // Collision の位置とサイズに合わせて描画
    Vector2d pos = transform->GetPosition() + m_drawOffset;
    // ★ Transform の scale をそのまま使う
    Vector2d scale = transform->GetScale();
    if (m_drawH > 0.0f && texH > 0)
    {
        float drawScale = m_drawH / static_cast<float>(texH);
        scale.x *= drawScale;
        scale.y *= drawScale;
    }
    else if (m_drawW > 0.0f && texW > 0)
    {
        float drawScale = m_drawW / static_cast<float>(texW);
        scale.x *= drawScale;
        scale.y *= drawScale;
    }

    renderer->DrawSpriteEx(
        pos, 
        scale.x,
        scale.y,
        0.0f, 
        m_handle,
        true,
        Vector2d((float)texW, (float)texH) * 0.5f,
        m_alpha,
        m_flipX,
        m_flipV,
        m_useCamera);
}

// --------------------
// 分割画像読み込み
// --------------------
bool SpriteComponent::LoadTextureDiv(const std::string& path, int xNum, int yNum)
{
    // まず画像全体を読み込み、サイズを取得
    int total = xNum * yNum;

    // =========================
    // キャッシュ確認
    // =========================
    auto it = s_textureDivCache.find(path);

    if (it != s_textureDivCache.end())
    {
        // すでに読み込み済み
        m_frames = it->second;

        if (m_frames.empty())
            return false;

        m_currentFrame = 0;
        m_handle = m_frames[0];

        int texW = 0;
        int texH = 0;

        GetGraphSize(m_handle, &texW, &texH);

        SetSize(
            static_cast<float>(texW),
            static_cast<float>(texH)
        );

        return true;
    }

    // =========================
    // まだキャッシュされていない
    // =========================
    int tempHandle = LoadGraph(path.c_str());
    if (tempHandle == -1) {
        std::cerr << "[ERROR] 画像読み込み失敗: " << path << std::endl;
        return false;
    }

    int texW = 0, texH = 0;
    GetGraphSize(tempHandle, &texW, &texH);
    DeleteGraph(tempHandle);

    // 各フレームの幅・高さ
    int frameW = texW / xNum;
    int frameH = texH / yNum;

    // 分割画像のハンドル
    std::vector<int> frames(total);

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        m_alpha
    );

    // 正しい幅・高さで分割読み込み
    int ret = LoadDivGraph(
        path.c_str(),
        total,
        xNum,
        yNum,
        frameW,
        frameH,
        frames.data()
    );
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        255
    );

    if (ret == -1) {
        std::cerr << "[ERROR] 分割画像読み込み失敗: " << path << std::endl;
        return false;
    }

    // =========================
    // キャッシュに保存
    // =========================
    s_textureDivCache[path] = frames;

    // =========================
    // このSpriteComponentで使用
    // =========================
    m_frames = frames;

    // 初期フレームを設定
    m_currentFrame = 0;
    m_handle = m_frames[0];

    SetSize((float)frameW, (float)frameH);

    std::cout << "分割画像読み込み成功: " << path
        << " (フレーム: " << total << ")" << std::endl;

    return true;
}

// --------------------
// 指定フレームに切替
// --------------------
void SpriteComponent::SetFrame(int index)
{
    if (index == -1) {
        m_handle = -1;
        return;
    }

    if (index < 0 || index >= (int)m_frames.size())
        return;

    m_currentFrame = index;
    m_handle = m_frames[index];

}

void SpriteComponent::SetEffectFrames(const std::vector<int>& frames)
{
    m_frames = frames;

    if (!m_frames.empty())
    {
        m_currentFrame = 0;
        m_handle = m_frames[0];
    }
}

// --------------------
// テクスチャを直接セット
// --------------------
void SpriteComponent::SetTexture(int handle)
{
    m_handle = handle;
}

// --------------------
// 単一画像読み込み
// --------------------
bool SpriteComponent::LoadTexture(const std::string& path) {
    auto it = s_textureCache.find(path);

    if (it != s_textureCache.end()) {
        m_handle = it->second;
    }
    else {
        m_handle = LoadGraph(path.c_str());
        if (m_handle == -1) {
            std::cerr << "[ERROR] 画像読み込み失敗:  " << path << std::endl;
            m_width = 32;
            m_height = 32;

            return true;
        }
        s_textureCache[path] = m_handle;
    }

    int w, h;
    GetGraphSize(m_handle, &w, &h);
    SetSize((float)w, (float)h);
    return true;
}

void SpriteComponent::SetSize(float w, float h) {
    m_width = w;
    m_height = h;
}

void SpriteComponent::ReleaseTextures()
{
    TextureManager::ReleaseTextures(
        s_textureCache
    );

    // 分割テクスチャ
    for (auto& pair : s_textureDivCache)
    {
        for (int handle : pair.second)
        {
            if (handle != -1)
            {
                DeleteGraph(handle);
            }
        }
    }

    s_textureDivCache.clear();
}

bool SpriteComponent::GetFlipH() const
{
    return m_flipX;
}