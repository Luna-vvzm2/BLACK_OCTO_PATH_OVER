#include "Scene.h"
#include "Actor.h"
#include "CollisionComponent.h"
#include <iostream>

Scene::Scene(Game* game)
	: m_game(game),
	m_isRunning(false),
	m_type(Type::Title)
{
}

Scene::~Scene() {
	ReleaseAllActors();
}

void Scene::AddActor(Actor* actor) {
	if (!actor) return;

	if (!actor->Init()) {
		std::cerr << "Scene::AddActor: Actor Init failed\n";
		delete actor;
		return;
	}

	m_actors.push_back(actor);
}

void Scene::AddUIActor(Actor* UIactor) {
	if (!UIactor) return;

	if (!UIactor->Init()) {
		std::cerr << "Scene::AddActor: Actor Init failed\n";
		delete UIactor;
		return;
	}

	m_UIactors.push_back(UIactor);
}

void Scene::AddBackActor(Actor* backactor) {
	if (!backactor) return;

	if (!backactor->Init()) {
		std::cerr << "Scene::AddActor: Actor Init failed\n";
		delete backactor;
		return;
	}

	m_backactors.push_back(backactor);
}

void Scene::RemoveDeadActors() {
	removeActors(m_actors);
	removeActors(m_UIactors);
	removeActors(m_backactors);
}

void Scene::ReleaseAllActors() {
	releaseActors(m_actors);
	releaseActors(m_UIactors);
	releaseActors(m_backactors);
}

