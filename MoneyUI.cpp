#include "MoneyUI.h"
#include "PlayerEntity.h"
#include "Game.h"
#include "Renderer.h"
#include "Color.h"

#include <DxLib.h>
#include <sstream>
#include <iostream>
#include <filesystem>

MoneyUI::MoneyUI(Scene* scene, PlayerEntity* player, const std::string& coinImagePath)
    : UIActor(scene)
    , m_player(player)
    , m_coinImagePath(coinImagePath)
{
    m_coinHandle = -1;
    m_visible = false;
    m_showTimer = 0.0f;
}

MoneyUI::~MoneyUI()
{
    if (m_coinHandle >= 0) {
        DeleteGraph(m_coinHandle);
        m_coinHandle = -1;
    }
}

bool MoneyUI::Init()
{
    if (!UIActor::Init()) return false;

    // 画像をロード（LoadGraph）。失敗してもフォールバックで矩形を描く
    m_coinHandle = LoadGraph(m_coinImagePath.c_str());
    if (m_coinHandle == -1) {
        // Try alternate path using current_path for debug
        try {
            auto alt = std::filesystem::current_path() / m_coinImagePath;
            std::cerr << "[MoneyUI] Try alt path: " << alt.string() << std::endl;
            m_coinHandle = LoadGraph(alt.string().c_str());
        }
        catch (...) {}
    }
    if (m_coinHandle == -1) {
        std::cerr << "[MoneyUI] Failed to load money image: " << m_coinImagePath << std::endl;
        // フォールバック: なしでも動く（四角を描く）
    }
    else {
        // Optionally we could get original size:
        int w = 0, h = 0;
        GetGraphSize(m_coinHandle, &w, &h);
        // If caller didn't change size, default to texture size
        if (m_imageW <= 0.0f) m_imageW = static_cast<float>(w);
        if (m_imageH <= 0.0f) m_imageH = static_cast<float>(h);
    }

    // Ensure UIActor's internal pos is consistent (m_posX/m_posY)
    m_posX = m_screenX;
    m_posY = m_screenY;

    return true;
}

void MoneyUI::Update(float deltaTime)
{
    // タイマー処理
    if (m_showTimer > 0.0f) {
        m_showTimer -= deltaTime;
        if (m_showTimer <= 0.0f) {
            m_showTimer = 0.0f;
            m_visible = false;
        }
    }

    UIActor::Update(deltaTime);
}

void MoneyUI::SetPosition(float x, float y)
{
    UIActor::SetPosition(x, y);
    m_screenX = x;
    m_screenY = y;
}

void MoneyUI::SetImageSize(float w, float h)
{
    m_imageW = w;
    m_imageH = h;
}

void MoneyUI::SetImageOffset(float offsetX, float offsetY)
{
    m_imageOffsetX = offsetX;
    m_imageOffsetY = offsetY;
}

void MoneyUI::ShowFor(float seconds)
{
    if (seconds <= 0.0f) {
        m_showTimer = -1.0f; // 無期限
    }
    else {
        m_showTimer = seconds;
    }
    m_visible = true;
}

void MoneyUI::Draw()
{
    if (!m_visible) return; // 非表示なら何もしない

    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    if (!renderer) return;

    float drawX = m_screenX + m_imageOffsetX;
    float drawY = m_screenY + m_imageOffsetY;

    // 右上アンカー処理: 画面幅を参照して drawX を上書き
    if (m_anchor == Anchor::TopRight) {
        float screenW = static_cast<float>(m_scene->GetGame()->GetWidth());
        // 画像の左上 X を計算: 画面右端 - marginRight - imageWidth
        drawX = screenW - m_marginRight - m_imageW + m_imageOffsetX;
        // Y は marginTop + imageOffsetY
        drawY = m_marginTop + m_imageOffsetY;
    }

    // 1) 画像描画（スクリーン座標固定）
    if (m_coinHandle >= 0) {
        DrawExtendGraph(
            static_cast<int>(drawX),
            static_cast<int>(drawY),
            static_cast<int>(drawX + m_imageW),
            static_cast<int>(drawY + m_imageH),
            m_coinHandle,
            TRUE
        );
    }
    else {
        renderer->DrawRect(Vector2d(drawX, drawY), m_imageW, m_imageH, Color(200, 180, 80), true, false);
    }

    // 2) 数字描画（右寄せ）: 画像の左端から gap 分左に文字の右端を合わせる
    if (!m_player) return;
    int money = m_player->GetMoney();
    std::ostringstream ss; ss << money;

    // テキストの右端位置 = drawX - m_gap
    float textRightX = drawX - m_gap;
    // 垂直中央合わせ
    float textY = drawY + (m_imageH - m_fontSize) * 0.5f;

    const std::string& font = m_scene->GetGame()->GatDebugFont();
    // DrawTextR は右寄せ描画 (pos.x が「右端位置」扱いになる)
    renderer->DrawTextR(Vector2d(textRightX, textY), ss.str(), Color(255, 235, 120), font, m_fontSize, false);
}