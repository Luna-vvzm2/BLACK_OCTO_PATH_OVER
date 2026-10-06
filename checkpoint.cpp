#include "Checkpoint.h"

#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"

#include <cmath>

Checkpoint::Checkpoint(Scene* scene, const Vector2d& position)
    : Actor(scene)
    , m_position(position)
    , m_sprite(nullptr)
    , m_isActivated(false)
{
}

bool Checkpoint::Init()
{
    auto* transform = AddComponent<TransformComponent>();
    m_sprite = AddComponent<SpriteComponent>("assets/images/blocks/checkpointPassive.png");

    if (!transform || !m_sprite)
    {
        return false;
    }

    transform->SetPosition(m_position);
    m_sprite->SetSize(104.0f, 104.0f);
    return true;
}

void Checkpoint::Update(float deltaTime)
{
    Actor::Update(deltaTime);

    if (m_sprite == nullptr)
    {
        return;
    }

    auto* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (playScene == nullptr)
    {
        return;
    }

    PlayerEntity* player = playScene->GetPlayer();
    if (player == nullptr)
    {
        return;
    }

    const Vector2d playerPosition = player->GetPos();
    constexpr float activationDistance = 52.0f;
    if (std::fabs(playerPosition.x - m_position.x) <= activationDistance &&
        std::fabs(playerPosition.y - m_position.y) <= activationDistance)
    {
        playScene->SetRespawnPosition(m_position);
    }

    const bool isCurrentCheckpoint = playScene->IsRespawnPosition(m_position);
    if (isCurrentCheckpoint != m_isActivated)
    {
        const char* texturePath = isCurrentCheckpoint
            ? "assets/images/blocks/checkpointActive.png"
            : "assets/images/blocks/checkpointPassive.png";
        m_sprite->LoadTexture(texturePath);
        m_isActivated = isCurrentCheckpoint;
    }
}
