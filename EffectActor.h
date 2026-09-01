#pragma once
#include "Actor.h"
#include "Vector2d.h"
#include "AnimationComponent.h"
#include <unordered_map>

struct EffectData
{
	std::vector<int> handles;

	AnimationClip clip;

	Vector2d size;
};

enum class EffectType
{
	WeakAttack1,
	WeakAttack2,
	WeakAttack3,
	WeakAttack4,
	StrongAttack1,
	StrongAttack2,
	WeakAirAttack1,
	WeakAirAttack2,
	WeakAirAttack3,
	Hayabusa,
	SquatAttack,
	ExecutionStart,
	ExecutionMove,
	ExecutionEnemy,
	Jump,
	JumpSecond,
};

class TransformComponent;
class SpriteComponent;



class EffectActor : public Actor {
public:
	explicit EffectActor(Scene* scene, const Vector2d& pos, EffectType effectType, bool flipX = false);
	virtual ~EffectActor() = default;

	bool Init() override;
	void Update(float deltaTime) override;

	ActorType GetType() const override
	{
		return ActorType::Effect;
	}

	void SetPos(const Vector2d& pos);
	Vector2d GetPos() const;

	void SetSize(const Vector2d& size);
	Vector2d GetSize() const;

	static bool LoadEffects();
	static const EffectData* GetEffectData(EffectType type);
	void SetFollowTarget(Actor* target, const Vector2d& offset);

protected:

	TransformComponent* m_transform;
	SpriteComponent* m_sprite;
	AnimationComponent* m_anim;
	EffectType m_effectType;

	Actor* m_followTarget = nullptr;
	Vector2d m_followOffset;

	bool m_flipX;

	Vector2d m_initialPos;
	Vector2d m_initialSize;

	static std::unordered_map< EffectType, EffectData > s_effects;
};

