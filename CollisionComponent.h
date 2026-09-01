#pragma once
#include "Vector2d.h"
#include "Component.h"
#include "Actor.h"
#include "TransformComponent.h"
#include <vector>

class Scene;
class Game;
class Renderer;

//	形状用クラス
enum class CollisionShape {
	None,	//	衝突判定無し,
	Circle,	//	円	
	Rect,		//	矩形
	Segment		//	線分
};

//	線分用構造体
struct Segment
{
	Vector2d start;
	Vector2d end;

	Segment() : start(Vector2d::Zero()), end(Vector2d::UnitX()) {}
	Segment(const Vector2d& s, const Vector2d& e) : start(s), end(e) {}

};

class CollisionComponent :public Component
{
public:
	explicit CollisionComponent(Actor* owner);
	~CollisionComponent() = default;

	bool Init() override { return true; }
	void Update(float deltaTime) override {}

	void SetNone();
	//	円形当たり判定の作成
	void SetCircle(float radius);
	//	矩形当たり判定の作成
	void SetRect(float width, float height);
	//	線分の当たり判定を作る
	void SetSegment(const Vector2d& start, const Vector2d& end);


	const Vector2d& GetOffset() const
	{
		return m_offset;
	}
	// 衝突判定
	// pos: このオブジェクトの座標
	// other: 判定対象のCollision
	// otherPos: 判定対象の座標
	bool CheckCollision(const CollisionComponent* other) const;

	// 点との衝突判定
	bool CheckPointCollision(const Vector2d& point) const;

	// 線分との衝突判定
	bool CheckSegmentCollision(const Segment& seg) const;

	std::vector<Segment> ConvertRectToSegments() const;

	// ゲッター
	CollisionShape GetShape() const { return m_shape; }
	float GetRadius() const { return m_radius; }
	float GetWidth() const { return m_width; }
	float GetHeight() const { return m_height; }
	Segment GetSegment() const { return m_segment; }
	void SetOffset(const Vector2d& offset)
	{
		m_offset = offset;
	}

	void DrawDebug() const;

private:
	//
	bool CheckCircleToCircle(const Vector2d& a, float ar, const Vector2d& b, float br) const;
	bool CheckRectToRect(const Vector2d& a, float aw, float ah, const Vector2d& b, float bw, float bh) const;
	bool CheckCircleToRect(const Vector2d& cpos, float cr, const Vector2d& rpos, float rw, float rh) const;
	bool CheckSegmentToCircle(const Segment& seg, const Vector2d& center, float radius) const;
	bool CheckSegmentToRect(const Segment& seg, const Vector2d& rpos, float rw, float rh) const;
	bool CheckPointInCircle(const Vector2d& point, const Vector2d& center, float radius) const;
	bool CheckPointInRect(const Vector2d& point, const Vector2d& center, float w, float h) const;
	float DistancePointToSegment(const Vector2d& point, const Segment& seg) const;
	bool DoSegmentsIntersect(const Segment& a, const Segment& b) const;
	Vector2d m_offset = Vector2d::Zero();
	CollisionShape m_shape;

	//	円形用情報
	float m_radius;

	//	矩形用情報
	float m_width;
	float m_height;

	//	線分用情報
	Segment m_segment;

	// 内部ユーティリティ
	Vector2d GetOwnerPosition() const
	{
		auto transform = m_owner->GetComponent<TransformComponent>();

		if (!transform)
			return Vector2d::Zero();

		return transform->GetPosition() + m_offset;
	}
};

