#include "Checkpoint.h"

#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"

#include <cmath>

Checkpoint::Checkpoint(Scene* scene, const Vector2d& pos)
    : EntityActor(scene, pos)
    , m_isActivated(false)
{
}

bool Checkpoint::Init()
{
    auto* transform = AddComponent<TransformComponent>();
    m_sprite = AddComponent<SpriteComponent>(GetTexturePath());

    if (!transform || !m_sprite)
    {
        return false;
    }

    transform->SetPosition(m_initialPos);
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
    if (std::fabs(playerPosition.x - m_initialPos.x) <= 52.0f &&
        playerPosition.y < m_initialPos.y)
    {
        playScene->SetRespawnPosition(m_initialPos);
    }

    const bool isCurrentCheckpoint = playScene->IsRespawnPosition(m_initialPos);
    if (isCurrentCheckpoint != m_isActivated)
    {
        const char* texturePath = isCurrentCheckpoint
            ? "assets/images/blocks/checkpointActive.png"
            : "assets/images/blocks/checkpointPassive.png";
        m_sprite->LoadTexture(texturePath);
        m_isActivated = isCurrentCheckpoint;
    }
}
