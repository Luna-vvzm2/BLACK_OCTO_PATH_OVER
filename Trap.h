#pragma once
#include "EntityActor.h" 
#include "Vector2d.h"

class CollisionComponent;

class Trap : public EntityActor { //EntityActorを継承してTrapクラスを定義
public:
	Trap(class Scene* scene, const Vector2d& pos, const Vector2d& size); //Trapを作るときはSceneのポインタと位置とサイズを受け取って初期化する
    ~Trap() override = default; //Trapが消えたら

    bool Init() override;
    void Update(float deltaTime) override;
    ActorType GetType() const override { return ActorType::Trap; }
    std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; }
private:
    int m_damage;             // プレイヤーに与えるダメージ量
    float m_currentCooldown;  // 現在のクールダウンタイマー
};