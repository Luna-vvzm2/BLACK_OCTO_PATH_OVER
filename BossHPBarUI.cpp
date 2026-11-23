#include "BossHPBarUI.h"

#include "Scene.h"
#include "Game.h"
#include "EnemyEntity.h"
#include "HPComponent.h"
#include "boss_01.h"
#include "boss_02.h"

#include <DxLib.h>
#include <algorithm>
#include <cmath>

namespace
{
constexpr int kImageWidth = 1418;
constexpr int kCropTop = 260;
constexpr int kCropHeight = 195;
constexpr float kBaseWidth = 610.0f;
constexpr float kBaseHeight = 84.0f;
}

BossHPBarUI::BossHPBarUI(Scene* scene) : UIActor(scene)
{
    SetName("BossHPBarUI");
}

BossHPBarUI::~BossHPBarUI()
{
    if (m_frameHandle >= 0) DeleteGraph(m_frameHandle);
}

bool BossHPBarUI::Init()
{
    if (!UIActor::Init()) return false;
    m_frameHandle = LoadGraph("assets/images/Enemy/Boss/enemyHP.png");
    return true;
}

void BossHPBarUI::Reset()
{
    m_activeBoss = nullptr;
    m_displayRatio = 1.0f;
    m_visible = false;
}

void BossHPBarUI::Update(float deltaTime)
{
    UIActor::Update(deltaTime);

    EnemyEntity* boss = nullptr;
    for (Actor* actor : m_scene->GetActors())
    {
        if (!actor || actor->GetState() != Actor::State::Active) continue;
        auto* candidate = dynamic_cast<EnemyEntity*>(actor);
        if (!candidate) continue;
        if (auto* first = dynamic_cast<Boss_01*>(candidate))
        {
            if (first->IsBattleFinished()) continue;
        }
        else if (auto* second = dynamic_cast<Boss_02*>(candidate))
        {
            if (second->IsBattleFinished()) continue;
        }
        else continue;

        if (candidate == m_activeBoss || candidate->GetChaseFlag())
        {
            boss = candidate;
            break;
        }
    }

    if (!boss || !boss->GetHP())
    {
        Reset();
        return;
    }

    const bool newEncounter = boss != m_activeBoss;
    m_activeBoss = boss;
    m_visible = true;

    int defeatHP = 0;
    if (auto* first = dynamic_cast<Boss_01*>(boss)) defeatHP = first->GetDefeatHP();
    const int battleHP = boss->GetHP()->GetMaxHP() - defeatHP;
    const float targetRatio = battleHP > 0
        ? std::clamp(static_cast<float>(boss->GetHP()->GetHP() - defeatHP) / battleHP, 0.0f, 1.0f)
        : 0.0f;

    if (newEncounter) m_displayRatio = targetRatio;
    const float speed = std::clamp(5.0f * deltaTime, 0.0f, 1.0f);
    m_displayRatio += (targetRatio - m_displayRatio) * speed;
    if (std::fabs(m_displayRatio - targetRatio) < 0.001f) m_displayRatio = targetRatio;
}

void BossHPBarUI::Draw()
{
    if (!m_visible || !m_scene || !m_scene->GetGame()) return;

    Game* game = m_scene->GetGame();
    const float sx = game->GetWidth() / 1280.0f;
    const float sy = game->GetHeight() / 720.0f;
    const float width = kBaseWidth * sx;
    const float height = kBaseHeight * sy;
    const float left = game->GetWidth() - width - 18.0f * sx;
    const float top = 0.0f;
    const auto tx = [&](float sourceX) { return static_cast<int>(left + sourceX * width / kImageWidth); };
    const auto ty = [&](float sourceY) { return static_cast<int>(top + (sourceY - kCropTop) * height / kCropHeight); };

    if (m_frameHandle >= 0)
    {
        DrawRectExtendGraph(tx(0), ty(kCropTop), tx(kImageWidth), ty(kCropTop + kCropHeight),
            0, kCropTop, kImageWidth, kCropHeight, m_frameHandle, TRUE);
    }
    else
    {
        DrawQuadrangle(tx(45), ty(290), tx(1375), ty(290),
            tx(1375), ty(402), tx(135), ty(402), GetColor(24, 24, 24), TRUE);
    }

    const float ratio = std::clamp(m_displayRatio, 0.0f, 1.0f);
    if (ratio <= 0.0f) return;

    // Fill toward the left, opposite the player's HP gauge. Keep the black frame visible.
    const float leftTop = 74.0f;
    const float leftBottom = 148.0f;
    const float right = 1358.0f;
    const float fillTop = right - (right - leftTop) * ratio;
    const float fillBottom = right - (right - leftBottom) * ratio;
    DrawQuadrangle(tx(fillTop), ty(301), tx(right), ty(301),
        tx(right), ty(391), tx(fillBottom), ty(391), GetColor(205, 35, 38), TRUE);
}
