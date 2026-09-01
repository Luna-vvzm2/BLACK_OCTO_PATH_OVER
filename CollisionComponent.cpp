#include "CollisionComponent.h"
#include "TransformComponent.h"
#include "Actor.h"
#include <cmath>
#include <algorithm>
#include "Renderer.h"
#include "Scene.h"
#include "Game.h"


CollisionComponent::CollisionComponent(Actor* owner)
	: Component(owner)
	, m_shape(CollisionShape::None)
	, m_radius(0.0f)
	, m_width(0.0f)
	, m_height(0.0f)
{
}
void CollisionComponent::SetNone() {
	m_shape = CollisionShape::None;
	m_radius = 0.0f;
	m_width = 0.0f;
	m_height = 0.0f;

}

void CollisionComponent::SetCircle(float radius) {
	m_shape = CollisionShape::Circle;
	m_radius = radius;
}

void CollisionComponent::SetRect(float width, float height) {
	m_shape = CollisionShape::Rect;
	m_width = width;
	m_height = height;
}

void CollisionComponent::SetSegment(const Vector2d& start, const Vector2d& end) {
	m_shape = CollisionShape::Segment;
	m_segment = Segment(start, end);
}

//	点との衝突判定
bool CollisionComponent::CheckPointCollision(const Vector2d& point) const {
	Vector2d pos = GetOwnerPosition();

	switch (m_shape) {
	case CollisionShape::Circle: {
		float distSq = (point - pos).lengthSq();
		return distSq <= (m_radius * m_radius);
	}
	case CollisionShape::Rect: {
		float halfW = m_width * 0.5f;
		float halfH = m_height * 0.5f;
		return (point.x >= pos.x - halfW && point.x <= pos.x + halfW &&
			point.y >= pos.y - halfH && point.y <= pos.y + halfH);
	}
	default:
		return false;
	}
}


//	線分との衝突判定

bool CollisionComponent::CheckSegmentCollision(const Segment& seg) const {
	Vector2d pos = GetOwnerPosition();
	if (m_shape == CollisionShape::Circle) {
		Vector2d segVec = seg.end - seg.start;
		Vector2d toCircle = pos - seg.start;
		float t = std::clamp(toCircle.dot(segVec) / segVec.lengthSq(), 0.0f, 1.0f);
		Vector2d closest = seg.start + segVec * t;
		return (pos - closest).lengthSq() <= (m_radius * m_radius);
	}
	if (m_shape == CollisionShape::Rect) {
		// 簡易判定: 線分の端点が矩形内にあるか
		return CheckPointCollision(seg.start) || CheckPointCollision(seg.end);
	}
	return false;
}

//	他のCollisionComponentとの衝突
bool CollisionComponent::CheckCollision(const CollisionComponent* other) const {
	if (other->GetShape() == CollisionShape::None || m_shape == CollisionShape::None) return false;
	if (!other) return false;

	Vector2d posA = GetOwnerPosition();
	Vector2d posB = other->GetOwnerPosition();

	CollisionShape shapeA = m_shape;
	CollisionShape shapeB = other->GetShape();

	// 両方Circle
	if (shapeA == CollisionShape::Circle && shapeB == CollisionShape::Circle) {
		return CheckCircleToCircle(posA, m_radius, posB, other->GetRadius());
	}

	// 両方Rect
	if (shapeA == CollisionShape::Rect && shapeB == CollisionShape::Rect) {
		return CheckRectToRect(posA, m_width, m_height, posB, other->GetWidth(), other->GetHeight());
	}

	// Circle vs Rect
	if (shapeA == CollisionShape::Circle && shapeB == CollisionShape::Rect) {
		return CheckCircleToRect(posA, m_radius, posB, other->GetWidth(), other->GetHeight());
	}

	// Rect vs Circle は対称性で反転
	if (shapeA == CollisionShape::Rect && shapeB == CollisionShape::Circle) {
		return CheckCircleToRect(posB, other->GetRadius(), posA, m_width, m_height);
	}
	//	線分と円形の判定
	if (shapeA == CollisionShape::Segment && shapeB == CollisionShape::Circle) {
		return CheckSegmentToCircle(m_segment, posB, other->GetRadius());

	} if (shapeA == CollisionShape::Segment && shapeB == CollisionShape::Rect) {
		return CheckSegmentToRect(m_segment, posB, other->GetWidth(), other->GetHeight());
	}
	//	線分と矩形の判定


	return false;
}


bool CollisionComponent::CheckCircleToCircle(const Vector2d& a, float ar, const Vector2d& b, float br) const {
	float r = ar + br;
	return (a - b).lengthSq() <= (r * r);
}

bool CollisionComponent::CheckRectToRect(const Vector2d& a, float aw, float ah, const Vector2d& b, float bw, float bh) const {
	float halfWA = aw * 0.5f;
	float halfHA = ah * 0.5f;
	float halfWB = bw * 0.5f;
	float halfHB = bh * 0.5f;
	return (std::abs(a.x - b.x) <= halfWA + halfWB) &&
		(std::abs(a.y - b.y) <= halfHA + halfHB);
}

bool CollisionComponent::CheckCircleToRect(const Vector2d& cpos, float cr, const Vector2d& rpos, float rw, float rh) const {
	Vector2d diff = cpos - rpos;
	float halfW = rw * 0.5f;
	float halfH = rh * 0.5f;
	float closestX = std::clamp(diff.x, -halfW, halfW);
	float closestY = std::clamp(diff.y, -halfH, halfH);
	Vector2d closest(closestX, closestY);
	return (diff - closest).lengthSq() <= (cr * cr);
}

bool CollisionComponent::CheckSegmentToCircle(const Segment& seg, const Vector2d& center, float radius) const {
	return DistancePointToSegment(center, seg) <= radius;
}

bool CollisionComponent::CheckSegmentToRect(const Segment& seg, const Vector2d& rpos, float rw, float rh) const {
	float halfW = rw * 0.5f;
	float halfH = rh * 0.5f;

	Vector2d topL = { rpos.x - halfW, rpos.y - halfH };
	Vector2d topR = { rpos.x + halfW, rpos.y - halfH };
	Vector2d bottomR = { rpos.x + halfW, rpos.y + halfH };
	Vector2d bottomL = { rpos.x - halfW, rpos.y + halfH };

	std::vector<Segment> edges = {
		{ topL, topR },
		{ topR, bottomR },
		{ bottomR, bottomL },
		{ bottomL, topL }
	};

	for (const auto& edge : edges) {
		if (DoSegmentsIntersect(seg, edge))
			return true;
	}

	for (const auto& edge : edges) {
		// 線分交差チェックを入れるべき（今は距離判定しかしてない）
		if (DistancePointToSegment(edge.start, seg) < 1e-5f ||
			DistancePointToSegment(edge.end, seg) < 1e-5f)
			return true;
	}
	if (CheckPointInRect(seg.start, rpos, rw, rh) || CheckPointInRect(seg.end, rpos, rw, rh)) {
		return true;
	}
	return false;
}

bool CollisionComponent::CheckPointInCircle(const Vector2d& point, const Vector2d& center, float radius) const {
	return (point - center).lengthSq() <= (radius * radius);
}

bool CollisionComponent::CheckPointInRect(const Vector2d& point, const Vector2d& center, float w, float h) const {
	float hw = w * 0.5f;
	float hh = h * 0.5f;
	return (point.x >= center.x - hw && point.x <= center.x + hw &&
		point.y >= center.y - hh && point.y <= center.y + hh);
}

float CollisionComponent::DistancePointToSegment(const Vector2d& point, const Segment& seg) const {
	Vector2d ab = seg.end - seg.start;
	Vector2d ap = point - seg.start;
	float t = std::clamp(ap.dot(ab) / ab.lengthSq(), 0.0f, 1.0f);
	Vector2d nearest = seg.start + ab * t;
	return (point - nearest).length();
}

std::vector<Segment> CollisionComponent::ConvertRectToSegments() const {
	std::vector<Segment> edges;
	if (m_shape != CollisionShape::Rect) return edges;

	Vector2d pos = GetOwnerPosition();
	float halfW = m_width * 0.5f;
	float halfH = m_height * 0.5f;

	Vector2d topL = { pos.x - halfW, pos.y - halfH };
	Vector2d topR = { pos.x + halfW, pos.y - halfH };
	Vector2d bottomL = { pos.x - halfW, pos.y + halfH };
	Vector2d bottomR = { pos.x + halfW, pos.y + halfH };

	edges.emplace_back(topL, topR);
	edges.emplace_back(topR, bottomR);
	edges.emplace_back(bottomR, bottomL);
	edges.emplace_back(bottomL, topL);

	return edges;
}

bool CollisionComponent::DoSegmentsIntersect(const Segment& a, const Segment& b) const {
	auto cross = [](const Vector2d& a, const Vector2d& b) { return a.x * b.y - a.y * b.x; };
	Vector2d r = a.end - a.start;
	Vector2d s = b.end - b.start;
	float denom = cross(r, s);
	if (fabs(denom) < 1e-8f) return false; // 平行

	Vector2d diff = b.start - a.start;
	float t = cross(diff, s) / denom;
	float u = cross(diff, r) / denom;
	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}




//	デバッグ描画
void CollisionComponent::DrawDebug() const {
#ifdef _DEBUG
	Renderer* renderer = m_owner->GetScene()->GetGame()->GetRenderer();
	if (!renderer) return;

	auto transform = m_owner->GetComponent<TransformComponent>();
	if (!transform) return;

	Vector2d pos = transform->GetPosition();



	switch (m_shape) {
	case CollisionShape::Rect:
		renderer->DrawRectCenter(pos, m_width, m_height, Color(0, 255, 0), false);
		break;
	case CollisionShape::Circle:
		renderer->DrawCircle(pos, m_radius, Color(0, 255, 0), false);
		break;
	case CollisionShape::Segment:
		renderer->DrawLine(m_segment.start, m_segment.end, Color(255, 255, 0));
		break;
	default:
		break;
	}
#endif
}