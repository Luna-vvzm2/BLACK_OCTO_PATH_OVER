#include "TrapEffect.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"

namespace
{
    constexpr float FRAME_TIME = 1.0f / 20.0f;   // 「フレーム数」の1フレームの長さ

    // 画像の表示サイズ(1.0 = シート1コマの元のサイズ)と、位置の補正
    constexpr float EFFECT_SCALE = 1.0f;

    // ---- 起動前(ループ)----
    const char* LOOP_SHEET_PATH = "assets/images/effects/loop_sheet.png";
    constexpr float LOOP_OFFSET_X = 0.0f;
    constexpr float LOOP_OFFSET_Y = 30.0f;
    constexpr int LOOP_SHEET_COLS = 3;
    constexpr int LOOP_SHEET_ROWS = 3;
    const std::vector<int> LOOP_FRAME_COUNTS = { 2, 2, 2, 3, 3, 3, 3, 2 };   // 各コマの表示フレーム数

    // ---- 起動時(1回)----
    const char* ON_SHEET_PATH = "assets/images/effects/on_sheet.png";
    constexpr float ON_OFFSET_X = 0.0f;
    constexpr float ON_OFFSET_Y = 0.0f;
    constexpr int ON_SHEET_COLS = 3;
    constexpr int ON_SHEET_ROWS = 3;
    const std::vector<int> ON_FRAME_COUNTS = { 3, 2, 2, 2, 1, 1, 1 };        // 各コマの表示フレーム数
}

TrapEffect::TrapEffect()
    : m_sprite(nullptr)
    , m_state(State::Loop)
    , m_loop(true)
    , m_frameIndex(0)
    , m_timer(0.0f)
{
}

bool TrapEffect::Init(SpriteComponent* sprite, TransformComponent* transform)
{
    m_sprite = sprite;
    m_state = State::Loop;
    if (!m_sprite) return false;

    if (!StartSheet(LOOP_SHEET_PATH, LOOP_SHEET_COLS, LOOP_SHEET_ROWS, LOOP_FRAME_COUNTS,
        true, LOOP_OFFSET_X, LOOP_OFFSET_Y))
    {
        return false;   // 画像が読み込めない場合は、罠の元のスプライトのまま
    }

    if (transform)
    {
        transform->SetScale({ EFFECT_SCALE, EFFECT_SCALE });
    }
    return true;
}

bool TrapEffect::StartSheet(const char* path, int cols, int rows, const std::vector<int>& frameCounts, bool loop,
    float offsetX, float offsetY)
{
    if (!m_sprite->LoadTextureDiv(path, cols, rows))
    {
        return false;
    }

    m_frameCounts = frameCounts;
    m_loop = loop;
    m_frameIndex = 0;
    m_timer = 0.0f;
    m_sprite->SetDrawOffset(offsetX, offsetY);   // シートごとのオフセット
    m_sprite->SetFrame(0);
    return true;
}

void TrapEffect::PlayActivate()
{
    if (m_state != State::Loop) return;     // 二重に呼ばれても1回だけ再生する

    if (m_sprite && StartSheet(ON_SHEET_PATH, ON_SHEET_COLS, ON_SHEET_ROWS, ON_FRAME_COUNTS, false,
        ON_OFFSET_X, ON_OFFSET_Y))
    {
        m_state = State::Activating;
    }
    else
    {
        m_state = State::Finished;          // 画像が読み込めない場合は、すぐ終了扱いにする(罠が消えなくなるのを防ぐ)
    }
}

void TrapEffect::Update(float deltaTime)
{
    if (!m_sprite || m_frameCounts.empty()) return;
    if (m_state == State::Finished) return;

    m_timer += deltaTime;

    // 経過時間に合わせて、コマを進める(1フレームに複数コマ進む場合にも対応)
    while (true)
    {
        float duration = m_frameCounts[m_frameIndex] * FRAME_TIME;
        if (duration <= 0.0f) duration = FRAME_TIME;
        if (m_timer < duration) break;

        m_timer -= duration;
        ++m_frameIndex;

        if (m_frameIndex >= static_cast<int>(m_frameCounts.size()))
        {
            if (m_loop)
            {
                m_frameIndex = 0;
            }
            else
            {
                // 1回再生が終わったら、画像を消して終了
                m_state = State::Finished;
                m_sprite->SetFrame(-1);
                return;
            }
        }
    }

    m_sprite->SetFrame(m_frameIndex);
}