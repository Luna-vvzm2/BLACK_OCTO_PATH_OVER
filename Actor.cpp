#include "Actor.h"
#include "Component.h"
#include "CollisionComponent.h"
#include "Scene.h"

Actor::Actor(Scene* scene)
	: m_scene(scene),
	m_state(State::Active)
{

}

Actor::~Actor() {
	for (auto comp : m_components) {
		delete comp;
	}
	m_components.clear();
}

void Actor::Update(float deltaTime) {

	if (m_state != State::Active) return;

	for (auto comp : m_components) {
		comp->Update(deltaTime);

	}
}

void Actor::Draw() {
	if (m_state == State::Dead) return;

	for (auto comp : m_components)
		comp->Draw();

#ifdef _DEBUG
	auto collision = GetComponent<CollisionComponent>();
	if (collision) collision->DrawDebug();
#endif
}

void Actor::SpawnEffect(Actor* effect) {
	if (m_scene)
		m_scene->SpawnActor(effect);
}

//	ÉOÉçÅ[ÉoÉãä÷êîóﬁ

void updateActors(std::vector<Actor*>& actors, float deltaTime) {
	size_t count = actors.size();

	for (size_t i = 0; i < count; ++i)
	{
		actors[i]->Update(deltaTime);
	}
}
void drawActors(std::vector<Actor*>& actors) {
	for (auto& actor : actors) {
		actor->Draw();
	}
}
void releaseActors(std::vector<Actor*>& actors) {
	for (auto& actor : actors) {
		delete actor;
	}
	actors.clear();
}
void removeActors(std::vector<Actor*>& actors) {

	auto it = actors.begin();
	while (it != actors.end()) {
		if ((*it)->IsDead()) {
			delete* it;
			it = actors.erase(it);
		}
		else {
			++it;
		}
	}
}