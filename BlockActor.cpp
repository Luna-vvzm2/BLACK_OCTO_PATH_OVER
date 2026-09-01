#include "BlockActor.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "CollisionComponent.h"

BlockActor::BlockActor(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: Actor(scene),
	m_transform(nullptr),
	m_sprite(nullptr),
	m_collision(nullptr),
	m_initialPos(pos),
	m_initialSize(colSize),
	m_texSize(texSize)
{
}

bool BlockActor::Init() {
	m_transform = AddComponent<TransformComponent>();
	m_sprite = AddComponent<SpriteComponent>(GetTexturePath());
	m_collision = AddComponent<CollisionComponent>();

	if (!m_sprite)
	{
		std::cout << "spriteŽ¸”s\n";
		return false;
	}

	m_transform->SetPosition(m_initialPos);
	m_transform->SetScale({ 1.0f, 1.0f });
	m_sprite->SetSize(m_texSize.x, m_texSize.y);
	m_collision->SetRect(m_initialSize.x, m_initialSize.y);
	return true;

}


void BlockActor::SetPos(const Vector2d& pos) {
	if (m_transform) m_transform->SetPosition(pos);
}

Vector2d BlockActor::GetPos() const {
	return m_transform ? m_transform->GetPosition() : Vector2d::Zero();
}

void BlockActor::SetSize(const Vector2d& size) {
	if (m_sprite) m_sprite->SetSize(size.x, size.y);
}

Vector2d BlockActor::GetSize() const {
	return m_sprite ? Vector2d(m_sprite->GetWidth(), m_sprite->GetHeight()) : Vector2d::Zero();
}