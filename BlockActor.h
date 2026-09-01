#pragma once
#include "Actor.h"
#include "Vector2d.h"

class TransformComponent;
class SpriteComponent;
class CollisionComponent;

enum class BlockType {
	Solid,      // í èÌÉuÉçÉbÉN
	Platform,    // Ç∑ÇËî≤ÇØë´èÍ
	StageExit
};

class BlockActor : public Actor
{
public:
	explicit BlockActor(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104,104 }, const Vector2d& texSize = { 104,104 });
	virtual ~BlockActor() override = default;

	bool Init() override;

	void SetPos(const Vector2d& pos);
	Vector2d GetPos() const;

	void SetSize(const Vector2d& size);
	Vector2d GetSize() const;

	ActorType GetType() const override { return ActorType::Block; }
	CollisionComponent* GetCollision() const { return m_collision; }

	virtual BlockType GetBlockType() const { return BlockType::Solid; }
protected:

	Vector2d m_initialPos;
	Vector2d m_initialSize;
	Vector2d m_texSize;

	TransformComponent* m_transform;
	SpriteComponent* m_sprite;
	CollisionComponent* m_collision;

	virtual std::string GetTexturePath() const = 0;
};