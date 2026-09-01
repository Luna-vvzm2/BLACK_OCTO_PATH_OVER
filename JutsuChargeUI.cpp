#include "JutsuChargeUI.h"
#include "Scene.h"
#include "Game.h"
#include "Renderer.h"
#include "SpriteComponent.h"
#include "PlayerEntity.h"

JutsuChargeUI::JutsuChargeUI(Scene* scene)
    : UIActor(scene)
    , m_damageCount(0)
{
}

bool JutsuChargeUI::Init()
{
    // 忍画像
    m_ninImage = AddComponent<SpriteComponent>();
    if (!m_ninImage) {
        std::cerr << "JutsuChargeUI::Init: Nin Image SpriteComponent creation failed\n";
        return false;
    }
    if (!m_ninImage->LoadTexture("assets/images/uies/nin.png")) {
        std::cerr << "ERROR: Failed to load nin.png\n";
    }

    printf("DEBUG: JutsuChargeUI initialized successfully\n");
    return true;
}

void JutsuChargeUI::Update(float deltaTime)
{
    UIActor::Update(deltaTime);

    if (m_scene == nullptr) {
        return;
    }

    PlayerEntity* player = nullptr;
    for (Actor* actor : m_scene->GetActors()) {
        if (actor != nullptr && actor->GetType() == ActorType::Player && !actor->IsDead()) {
            player = static_cast<PlayerEntity*>(actor);
            break;
        }
    }

    if (player == nullptr) {
        return;
    }

    int jutsuGaugeAmount = player->GetJutsuGaugeAmount();
    m_jutsuchage = player -> GetJutsuCharge();
}

void JutsuChargeUI::Draw()
{
    if (IsDead()) {
        return;
    }

    Game* game = m_scene->GetGame();
    if (game == nullptr) {
        return;
    }

    Renderer* renderer = game->GetRenderer();
    if (renderer == nullptr) {
        return;
    }

    bool shouldDisplay = (m_jutsuchage && m_isVisible);

    if (shouldDisplay && m_ninImage != nullptr) {
        int ninHandle = m_ninImage->GetHandle();

        if (ninHandle >= 0) {
            // ★変更：スケールを適用
            renderer->DrawSpriteEx(
                Vector2d(m_ninImageX, m_ninImageY),
                m_ninImageScale,  // X方向のスケール
                m_ninImageScale,  // Y方向のスケール
                0.0f,
                ninHandle,
                true,
                Vector2d::Zero(),
                255,
                false,
                false,
                false
            );
        }
        else {
            printf("DEBUG: nin image handle is invalid: %d\n", ninHandle);
        }
    }
}

void JutsuChargeUI::SetNinImagePosition(float x, float y)
{
    m_ninImageX = x;
    m_ninImageY = y;
    printf("DEBUG: Nin image position set to (%.1f, %.1f)\n", x, y);
}

void JutsuChargeUI::SetNinImageScale(float scale)
{
    m_ninImageScale = scale;
    printf("DEBUG: Nin image scale set to %.2f\n", scale);
}