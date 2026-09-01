#include "EntityActor.h"
#include "PlayScene.h"
#include "BlockActor.h"
#include "StageBackActor.h"
#include "StageExitActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "SpriteComponent.h"
#include "CollisionComponent.h"
#include <DxLib.h>

EntityActor::EntityActor(Scene* scene, const Vector2d& pos, const Vector2d& size)
	: Actor(scene),
	m_transform(nullptr),
	m_velocity(nullptr),
	m_sprite(nullptr),
	m_collision(nullptr),
	m_initialPos(pos),
	m_initialVel( { 0,0 } ),
	m_initialSize(size),
	m_state(ActionState::IDLE),
	m_isGround(false)
{
}

bool EntityActor::Init() {
	m_transform = AddComponent<TransformComponent>();
	m_velocity = AddComponent<VelocityComponent>();
	m_sprite = AddComponent<SpriteComponent>(GetTexturePath());
	m_collision = AddComponent<CollisionComponent>();

	m_transform->SetPosition(m_initialPos);
	m_velocity->SetVelocity(m_initialVel);
	m_sprite->SetSize(m_initialSize.x, m_initialSize.y);
	m_collision->SetRect(m_initialSize.x, m_initialSize.y);
    
   
	return true;

}

void EntityActor::SetPos(const Vector2d& pos) {
	if (m_transform) m_transform->SetPosition(pos);
}

Vector2d EntityActor::GetPos() const {
	return m_transform ? m_transform->GetPosition() : Vector2d::Zero();
}

void EntityActor::SetVel(const Vector2d& vel) {
	if (m_velocity) m_velocity->SetVelocity(vel);
}

Vector2d EntityActor::GetVel() const {
	return m_velocity ? m_velocity->GetVelocity() : Vector2d::Zero();
}

void EntityActor::SetSize(const Vector2d& size) {
	if (m_sprite) m_sprite->SetSize(size.x, size.y);
}

Vector2d EntityActor::GetSize() const {
	return m_sprite ? Vector2d(m_sprite->GetWidth(), m_sprite->GetHeight()) : Vector2d::Zero();
}

void EntityActor::MoveAndCollide(float deltaTime) {
    m_isGround = false;

    Vector2d pos = m_transform->GetPosition();
    Vector2d vel = m_velocity->Get();

    // === Y方向移動 ===
    pos.y += vel.y * deltaTime;
    m_transform->SetPosition(pos);

    // === Y方向衝突処理 ===
    for (auto actor : m_scene->GetActors()) {
        Vector2d actorPos;
        CollisionComponent* actorCol = nullptr;
        BlockActor* block = nullptr;

        switch (actor->GetType()) {
        case ActorType::Block:
            block = static_cast<BlockActor*>(actor);
            actorCol = block->GetCollision();
            actorPos = block->GetPos();
            if (std::abs(actorPos.x - pos.x) > 500)
                continue;

            if (std::abs(actorPos.y - pos.y) > 300)
                continue;
            break;
        case ActorType::StageExit:
        {
            auto exit = static_cast<StageExitActor*>(actor);
            if (GetType() != ActorType::Player) continue;
            CollisionComponent* exitCol = exit->GetCollision();

            Vector2d exitPos = exit->GetPos();

            float halfW = m_collision->GetWidth() * 0.5f;
            float halfH = m_collision->GetHeight() * 0.5f;
            float exitHalfW = exitCol->GetWidth() * 0.5f;
            float exitHalfH = exitCol->GetHeight() * 0.5f;

            Vector2d playerPos = m_transform->GetPosition();

            if (std::abs(playerPos.x - exitPos.x) < halfW + exitHalfW &&
                std::abs(playerPos.y - exitPos.y) < halfH + exitHalfH)
            {
                static_cast<PlayScene*>(m_scene)->RequestStageChange(exit->GetNextStage(), 0);
            }
            continue;
        } break;
        case ActorType::StageBack:
        {
            auto back = static_cast<StageBackActor*>(actor);
            if (GetType() != ActorType::Player) continue;
            CollisionComponent* backCol = back->GetCollision();

            Vector2d backPos = back->GetPos();

            float halfW = m_collision->GetWidth() * 0.5f;
            float halfH = m_collision->GetHeight() * 0.5f;
            float backHalfW = backCol->GetWidth() * 0.5f;
            float backHalfH = backCol->GetHeight() * 0.5f;

            Vector2d playerPos = m_transform->GetPosition();

            if (std::abs(playerPos.x - backPos.x) < halfW + backHalfW &&
                std::abs(playerPos.y - backPos.y) < halfH + backHalfH)
            {
                static_cast<PlayScene*>(m_scene)->RequestStageChange(
                    back->GetPrevStage(),
                    back->GetSpawnIndex());
            }

            continue;
        }
        break;
        default:
            continue;
        }

        Vector2d playerPos = m_transform->GetPosition();
        float halfW = m_collision->GetWidth() * 0.5f;
        float halfH = m_collision->GetHeight() * 0.5f;
        float aHalfW = actorCol->GetWidth() * 0.5f;
        float aHalfH = actorCol->GetHeight() * 0.5f;

        float diffX = playerPos.x - actorPos.x;
        float diffY = playerPos.y - actorPos.y;
        float overlapX = (halfW + aHalfW) - std::abs(diffX);
        float overlapY = (halfH + aHalfH) - std::abs(diffY);

        if (overlapY > 0 && overlapX > 0) {
            if (overlapY < overlapX) {
                if (diffY > 0) { // 下から当たった
                    if (block->GetBlockType() != BlockType::Platform) {
                        playerPos.y = actorPos.y + aHalfH + halfH;
                        vel.y = 0;
                    }
                }
                else { // 上に乗った
                    bool ignorePlatform = false;
                    if (block->GetBlockType() == BlockType::Platform)
                    {
                        if (IgnorePlatform())
                        {
                            ignorePlatform = true;
                        }
                    }

                    if (!ignorePlatform)
                    {
                        playerPos.y = actorPos.y - aHalfH - halfH;
                        vel.y = 0;

                        m_isGround = true;
                    }
                }
            }
            m_transform->SetPosition(playerPos);
        }
    }

    // === X方向移動 ===
    pos = m_transform->GetPosition();
    pos.x += vel.x * deltaTime;
    m_transform->SetPosition(pos);

    // === X方向衝突処理 ===
    for (auto actor : m_scene->GetActors()) {
        Vector2d actorPos;
        CollisionComponent* actorCol = nullptr;
        BlockActor* block = nullptr;

        switch (actor->GetType()) {
        case ActorType::Block:
            block = static_cast<BlockActor*>(actor);
            actorCol = block->GetCollision();
            actorPos = block->GetPos();
            break;
        default:
            continue;
        }

        Vector2d playerPos = m_transform->GetPosition();
        float halfW = m_collision->GetWidth() * 0.5f;
        float halfH = m_collision->GetHeight() * 0.5f;
        float aHalfW = actorCol->GetWidth() * 0.5f;
        float aHalfH = actorCol->GetHeight() * 0.5f;

        float diffX = playerPos.x - actorPos.x;
        float diffY = playerPos.y - actorPos.y;
        float overlapX = (halfW + aHalfW) - std::abs(diffX);
        float overlapY = (halfH + aHalfH) - std::abs(diffY);

        if (overlapX > 0 && overlapY > 0) {
            // === X方向補正 ===
            if (overlapX < overlapY) {
                if (diffX > 0) { // ブロック右側（左に押し戻す）
                    playerPos.x = actorPos.x + aHalfW + halfW;
                }
                else { // 左側（右に押し戻す）
                    playerPos.x = actorPos.x - aHalfW - halfW;
                }
            }
            vel.x = 0;
            m_transform->SetPosition(playerPos);
        }
    }
    m_velocity->Set(vel);
}

//追加
//void EntityActor::Update(float deltaTime)
//{
//    MoveAndCollide(deltaTime);
//}



