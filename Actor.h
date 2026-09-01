#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <utility>
#include <iostream>

class Scene;
class Component;

enum class ActorType {
	Player,
	Block,
	Ball,
	Kunai,
	Enemy,
	Effect,
	StageBack,
	StageExit,
	TreasureBox,
	Trap,
	UI,
	DropItem
};

class Actor {
public:
	enum class State {
		Active,
		Paused,
		Dead
	};

	explicit Actor(class Scene* scene);
	virtual ~Actor();

	virtual bool Init() { return true; }
	virtual void Update(float deltaTime);
	virtual void Draw();

	virtual ActorType GetType() const = 0;

	Scene* GetScene() { return m_scene; }

	// 状態管理
	State GetState() const { return m_state; }
	void SetState(State state) { m_state = state; }

	bool IsDead() const { return m_state == State::Dead; }

	// 識別用
	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }
	void SpawnEffect(Actor* effect);

	// コンポーネント管理
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args);

	template<typename T>
	T* GetComponent();

protected:
	Scene* m_scene;
	State m_state;
	std::string m_name;

	std::vector<Component*> m_components;
};

// グローバル関数群
void updateActors(std::vector<Actor*>& actors, float deltaTime);
void drawActors(std::vector<Actor*>& actors);
void releaseActors(std::vector<Actor*>& actors);
void removeActors(std::vector<Actor*>& actors);

//テンプレート実装
template<typename T, typename... Args>
T* Actor::AddComponent(Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

	// コンポーネント生成
	T* comp = new T(this, std::forward<Args>(args)...);

	if (!comp->Init()) {
		std::cerr << "[ERROE]コンポーネント初期化失敗: " << typeid(T).name() << std::endl;
		delete comp;
		return nullptr;
	}

	m_components.push_back(comp);
	return comp;
}

template<typename T>
T* Actor::GetComponent() {
	for (auto comp : m_components) {
		if (auto c = dynamic_cast<T*>(comp)) return c;
	}

	return nullptr;
}

