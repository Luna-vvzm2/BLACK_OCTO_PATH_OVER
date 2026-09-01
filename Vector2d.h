#pragma once
#include <cmath>

struct Vector2d {
	float x{};
	float y{};

	//	引数なしコンストラクタ
	Vector2d() : x(0), y(0) {}
	//	引数ありコンストラクタ
	Vector2d(float x, float y) : x(x), y(y) {}

	//	基本演算
	inline Vector2d operator+(const Vector2d& other) const { return { x + other.x, y + other.y }; }
	inline Vector2d operator-(const Vector2d& other) const { return { x - other.x, y - other.y }; }
	inline Vector2d operator*(float s) const { return { x * s, y * s }; }
	inline Vector2d operator/(float s) const { return { x / s, y / s }; }

	inline Vector2d& operator+=(const Vector2d& other) { x += other.x; y += other.y; return *this; }
	inline Vector2d& operator-=(const Vector2d& other) { x -= other.x; y -= other.y; return *this; }

	//	ベクトル演算
	//	ベクトルの長さ(大きさ)計算
	inline float length() const { return std::sqrt(x * x + y * y); }
	//	ベクトルの長さの二乗計算
	inline float lengthSq() const { return x * x + y * y; }
	//	ベクトルの正規化(方向ベクトル)を計算
	inline Vector2d normalize() const {
		float len = length();
		return (len > 0.0f) ? Vector2d(x / len, y / len) : Vector2d(0, 0);

	}
	//	二つのベクトルの内積
	inline float dot(const Vector2d& other) const { return x * other.x + y * other.y; }
	//	二つのベクトルの外積
	inline float cross(const Vector2d& other) const { return x * other.y - y * other.x; }

	//	定数ベクトル
	inline static Vector2d Zero() { return { 0.0f, 0.0f }; }
	inline static Vector2d One() { return { 1.0f, 1.0f }; }
	inline static Vector2d UnitX() { return { 1.0f, 0.0f }; }
	inline static Vector2d UnitY() { return { 0.0f, 1.0f }; }

	inline bool operator==(const Vector2d& other) const { return x == other.x && y == other.y; }
	inline bool operator!=(const Vector2d& other) const { return !(*this == other); }
	inline bool IsNear(const Vector2d& other, float eps = 1e-5f) const {
		return std::fabs(x - other.x) < eps && std::fabs(y - other.y) < eps;
	}
};
