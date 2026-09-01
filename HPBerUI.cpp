#include "HPBarUI.h"
#include "HPComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include <iostream>

static const std::string DEFAULT_BAR = "assets/images/uies/hpBar.png";
static const std::string DEFAULT_FRAME = "assets/images/uies/hpBarFrame.png";

HPBarUI::HPBarUI(Scene* scene, HPComponent* hpComp)
    : UIActor(scene)
    , m_hp(hpComp)
    , m_maxWidth(180.0f)
    , m_height(12.0f)
{
    m_transform = AddComponent<TransformComponent>();
    m_backBar = AddComponent<SpriteComponent>("assets/images/uies/hpBarFrame.png", false);
    // m_sprite = AddComponent<SpriteComponent>("assets/images/uies/hpBar.png");

    SetName("HPBarUI");
}

bool HPBarUI::Init()
{
    if (!UIActor::Init()) return false;

    // 左上に小さく配置
    m_transform->SetPosition({ 0, -120 });

    // バーの最大幅（画像に合わせて調整）
    //m_maxWidth = 60.0f;   // ← 小さくする

    return true;
}


void HPBarUI::Update(float deltaTime)
{
    UIActor::Update(deltaTime);

    if (m_hp)
    {
        // 【滑らかに減らす演出を入れる場合】
        float targetRatio = (float)m_hp->GetHP() / m_hp->GetMaxHP();
        m_displayRatio += (targetRatio - m_displayRatio) * 5.0f * deltaTime;
        if (fabs(m_displayRatio - targetRatio) < 0.001f) m_displayRatio = targetRatio;
    }
}

void HPBarUI::Draw() {
    if (!m_isVisible || !m_backBar || !m_hp) return;

    Vector2d pos = m_transform->GetPosition();
    float scale = 0.7f;

    // --- フレーム描画 ---
    m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
        pos,
        scale, scale,
        0.0f,
        m_backBar->GetHandle(),
        true,
        Vector2d(0, 0),
        255,
        false, false, false
    );

    // --- HP割合の計算（★ここを m_displayRatio に変更） ---
    // これにより、実際のHPが減ってもゲージの描画幅がスルスルと滑らかに縮みます
    float barWidth = m_maxWidth * m_displayRatio;

    int skew = 14; // ★斜めの傾き具合（ドット数）。数値を大きくするともっと傾きます。

    // 基本となる左上の開始位置
    int baseLeft = static_cast<int>(pos.x + 135);
    int baseTop = static_cast<int>(pos.y + 165);

    // 4頂点の座標を計算（時計回りに指定します：左上 -> 右上 -> 右下 -> 左下）
    int x1 = baseLeft;                 // 1. 左上 (右にずらす)
    int y1 = baseTop;

    int x2 = static_cast<int>(baseLeft + barWidth); // 2. 右上
    int y2 = baseTop;

    int x3 = static_cast<int>(baseLeft + barWidth - skew); // 3. 右下
    if (x3 < baseLeft + skew) {
		x3 = baseLeft + skew;
    }
    int y3 = static_cast<int>(baseTop + m_height);

    int x4 = baseLeft + skew;                        // 4. 左下 (ずらさない)
    int y4 = y3;

    // HPが0より大きいときだけ描画（幅が0のときの誤描画を防ぐ安全策）
    if (barWidth > 0.0f) {
        // DrawQuadrangle(左上X, 左上Y, 右上X, 右上Y, 右下X, 右下Y, 左下X, 左下Y, 色, 塗りつぶしフラグ)
        DrawQuadrangle(x1, y1, x2, y2, x3, y3, x4, y4, GetColor(255, 0, 0), TRUE);
    }
}

void HPBarUI::SetPosition(float x, float y)
{
    if (m_transform)
        m_transform->SetPosition({ x, y });
}

void HPBarUI::SetBarSize(float w, float h)
{
    m_maxWidth = w;
    m_height = h;
}