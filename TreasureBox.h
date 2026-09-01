#pragma once
#include "DropData.h"
#include "EnemyEntity.h"

class TreasureBoxEntity : public EnemyEntity
{
public:
	TreasureBoxEntity(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime);
    void OnDead();
    std::string GetTexturePath() const override;
    ActorType GetType() const override { return ActorType::TreasureBox; }
    void Open() { m_isOpened = true; }
private:

    bool m_isOpened;
    float m_damageTimer = 0.0f;
    int m_damagePerSecond = 10;
    bool m_isFading = false;
    float m_alpha = 255.0f;
    // フェード開始までの待ち時間
    float m_fadeDelay = 0.2f;
    float m_fadeTimer = 2.0f;
    // フェードにかける時間（秒）
    float m_fadeTime = 0.8f;

    float m_fadeSpeed = 100.0f;
};