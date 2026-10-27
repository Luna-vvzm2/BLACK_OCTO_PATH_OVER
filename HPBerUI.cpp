
#include "HPBarUI.h"
#include "HPComponent.h"
#include "TransformComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Game.h"

#include <DxLib.h>
#include <cmath>

// 新しいHP・影ゲージ画像
static const char* STATUS_BAR_IMAGE =
"assets/images/uies/hp.png";

// 元画像サイズ
static constexpr int IMAGE_WIDTH = 1418;

// 画像の上下の余白をカット
static constexpr int CROP_TOP = 260;
static constexpr int CROP_HEIGHT = 270;

HPBarUI::HPBarUI(Scene* scene, HPComponent* hpComp)
    : UIActor(scene)
    , m_hp(hpComp)
    , m_maxWidth(610.0f)
    , m_height(34.0f)
{
    m_transform = AddComponent<TransformComponent>();

    SetName("HPBarUI");
}

HPBarUI::~HPBarUI()
{
    if (m_frameHandle >= 0)
    {
        DeleteGraph(m_frameHandle);
        m_frameHandle = -1;
    }
}

bool HPBarUI::Init()
{
    if (!UIActor::Init())
        return false;

    // 背景画像を読み込み
    m_frameHandle = LoadGraph(STATUS_BAR_IMAGE);

    if (m_frameHandle < 0)
        return false;

    // 左上を基準に配置
    m_transform->SetPosition({ 0.0f, 0.0f });

    return true;
}

void HPBarUI::Update(float deltaTime)
{
    UIActor::Update(deltaTime);

    // 影ゲージの波を動かす
    m_shadowWaveTime += deltaTime;

    if (!m_hp)
        return;

    const float maxHP =
        static_cast<float>(m_hp->GetMaxHP());

    float targetRatio = 0.0f;

    if (maxHP > 0.0f)
    {
        targetRatio =
            static_cast<float>(m_hp->GetHP()) / maxHP;
    }

    if (targetRatio < 0.0f)
        targetRatio = 0.0f;

    if (targetRatio > 1.0f)
        targetRatio = 1.0f;

    // HPの滑らかな減少演出
    float speed = 5.0f * deltaTime;

    if (speed < 0.0f)
        speed = 0.0f;

    if (speed > 1.0f)
        speed = 1.0f;

    m_displayRatio +=
        (targetRatio - m_displayRatio) * speed;

    if (std::fabs(m_displayRatio - targetRatio) < 0.001f)
    {
        m_displayRatio = targetRatio;
    }
}

void HPBarUI::Draw()
{
    if (!m_isVisible || !m_hp || !m_transform)
        return;

    if (!m_scene)
        return;

    Game* game = m_scene->GetGame();

    if (!game || m_frameHandle < 0)
        return;

    PlayScene* playScene =
        dynamic_cast<PlayScene*>(m_scene);

    if (!playScene)
        return;

    PlayerEntity* player = playScene->GetPlayer();

    if (!player)
        return;

    // ======================================
    // 画面サイズに対応
    // ======================================

    const float screenScaleX =
        game->GetWidth() / 1280.0f;

    const float screenScaleY =
        game->GetHeight() / 720.0f;

    // HPバーの幅と高さに合わせて
    // 元画像の表示倍率を決定
    const float sx =
        (m_maxWidth / 1306.0f) * screenScaleX;

    const float sy =
        (m_height / 73.0f) * screenScaleY;

    Vector2d pos = m_transform->GetPosition();

    const float left = pos.x * screenScaleX;
    const float top = pos.y * screenScaleY;

    // 元画像の座標を画面座標に変換
    auto TX = [&](float x) -> int
        {
            return static_cast<int>(left + x * sx);
        };

    auto TY = [&](float y) -> int
        {
            return static_cast<int>(
                top + (y - CROP_TOP) * sy
                );
        };

    // ======================================
    // ① 背景画像を描画
    // ======================================

    DrawRectExtendGraph(
        TX(0),
        TY(CROP_TOP),
        TX(IMAGE_WIDTH),
        TY(CROP_TOP + CROP_HEIGHT),
        0,
        CROP_TOP,
        IMAGE_WIDTH,
        CROP_HEIGHT,
        m_frameHandle,
        TRUE
    );

    // ======================================
    // ② HPゲージ（緑）
    // ======================================

    float hpRatio = m_displayRatio;

    if (hpRatio < 0.0f) hpRatio = 0.0f;
    if (hpRatio > 1.0f) hpRatio = 1.0f;

    // 元画像内のHPバー内側の座標
    const float hpLeft = 4.0f;
    const float hpTop = 311.0f;
    const float hpBottom = 384.0f;

    // 右側は斜めの形状
    const float hpRightTop = 1309.0f;
    const float hpRightBottom = 1257.0f;

    if (hpRatio > 0.0f)
    {
        const float fillRightTop =
            hpLeft +
            (hpRightTop - hpLeft) * hpRatio;

        const float fillRightBottom =
            hpLeft +
            (hpRightBottom - hpLeft) * hpRatio;

        DrawQuadrangle(
            TX(hpLeft),
            TY(hpTop),

            TX(fillRightTop),
            TY(hpTop),

            TX(fillRightBottom),
            TY(hpBottom),

            TX(hpLeft),
            TY(hpBottom),

            GetColor(0, 210, 65),
            TRUE
        );
    }

    // ======================================
    // ③ 影ゲージ（黒）
    // ======================================

    const float shadow =
        static_cast<float>(player->GetShadowGauge());

    const float maxShadow =
        static_cast<float>(player->GetShadowGaugeMax());

    float shadowRatio = 0.0f;

    if (maxShadow > 0.0f)
    {
        shadowRatio = shadow / maxShadow;
    }

    if (shadowRatio < 0.0f)
        shadowRatio = 0.0f;

    if (shadowRatio > 1.0f)
        shadowRatio = 1.0f;

    // 元画像内の影ゲージ内側の座標
    const float shadowLeft = 4.0f;
    const float shadowTop = 390.0f;
    const float shadowBottom = 432.0f;

    const float shadowRightTop = 1033.0f;
    const float shadowRightBottom = 1004.0f;


    if (shadowRatio > 0.0f)
    {
        // =====================================
        // 影ゲージの波アニメーション
        // =====================================

        // 波の設定
        const float waveAmplitude = 7.0f;  // 波の高さ
        const float waveFrequency = 0.028f; // 波の細かさ
        const float waveSpeed = 3.0f;      // 波の移動速度

        // 波の中心位置
        const float waveBaseY = shadowTop + 9.0f;

        // ゲージの右端位置
        const float fillRightTop =
            shadowLeft +
            (shadowRightTop - shadowLeft) * shadowRatio;

        const float fillRightBottom =
            shadowLeft +
            (shadowRightBottom - shadowLeft) * shadowRatio;

        // 波を細かい四角形に分割して描画
        constexpr int SEGMENTS = 80;

        for (int i = 0; i < SEGMENTS; ++i)
        {
            float t1 =
                static_cast<float>(i) / SEGMENTS;

            float t2 =
                static_cast<float>(i + 1) / SEGMENTS;

            // 上側のX座標
            float x1 = shadowLeft +
                (fillRightTop - shadowLeft) * t1;

            float x2 = shadowLeft +
                (fillRightTop - shadowLeft) * t2;

            // 下側のX座標
            float bottomX1 = shadowLeft +
                (fillRightBottom - shadowLeft) * t1;

            float bottomX2 = shadowLeft +
                (fillRightBottom - shadowLeft) * t2;

            // sin関数で波を生成
            float y1 = waveBaseY +
                std::sin(
                    x1 * waveFrequency -
                    m_shadowWaveTime * waveSpeed
                ) * waveAmplitude;

            float y2 = waveBaseY +
                std::sin(
                    x2 * waveFrequency -
                    m_shadowWaveTime * waveSpeed
                ) * waveAmplitude;

            // 波の表面から下側まで黒く塗る
            DrawQuadrangle(
                TX(x1), TY(y1),
                TX(x2), TY(y2),
                TX(bottomX2), TY(shadowBottom),
                TX(bottomX1), TY(shadowBottom),
                GetColor(0, 0, 0),
                TRUE
            );
        }
    }

}

void HPBarUI::SetPosition(float x, float y)
{
    if (m_transform)
    {
        m_transform->SetPosition({ x, y });
    }
}

void HPBarUI::SetBarSize(float w, float h)
{
    m_maxWidth = w;
    m_height = h;
}
