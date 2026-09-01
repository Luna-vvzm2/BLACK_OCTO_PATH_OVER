#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include "Actor.h"

class Game;
class Actor;
struct Segment;

class Scene
{
public:
	/*
	Scene / Actor 管理ルール

	1.	Scene は Actor の所有者である。
		- Scene が破棄されると全 Actor も delete される。
	2.	Actor の追加は AddActor(Actor*) で行う。
	*/

	enum class Type {
		Title,
		Play,
		Clear, //クリアシーンのために追加

	};
	//addactorを移動
	void AddActor(Actor* actor);
	explicit Scene(class Game* game);
	virtual ~Scene();

	virtual bool Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;

	Game* GetGame() { return m_game; }
	bool IsRunning() const { return m_isRunning; }
	Type GetType() const { return m_type; }
	const std::vector<Actor*>& GetActors() const { return m_actors; }

	void SpawnActor(Actor* actor) { AddActor(actor); }
	void SpawnUIActor(Actor* actor) { AddUIActor(actor); }



protected:
	
	void AddUIActor(Actor* UIactor);
	void AddBackActor(Actor* backactor);
	void RemoveDeadActors();   // 死んだActorを消す
	void ReleaseAllActors();  // 全Actorを消す

	Game* m_game;
	bool m_isRunning;
	Type m_type;
	std::vector<Actor*> m_actors;  // 生ポインタで保持
	std::vector<Actor*> m_UIactors;  // 生ポインタで保持
	std::vector<Actor*> m_backactors;

	std::vector<Segment> m_outerSegments;
};

