#pragma once

#include "EnemyBullet.h"

class AnimationComponent;


class SpiritBulletEntity : public EnemyBullet
{
public:
    SpiritBulletEntity(
        Scene* scene,
        const Vector2d& pos,
        const Vector2d& velocity,
        float deleteRange,
        const std::string& texturePath,
        int damage = 10,
        const Vector2d& drawSize = Vector2d::Zero(),
        bool rotate = false,
        float rotateInterval = 0.0f,
        float rotateStep = 0.0f
    );

    bool Init() override;
    protected:
    bool UseBlockCollision() const override { return true; }
    AnimationComponent* m_animation;


    void UpdateAnimation();
private:
    bool m_faceLeft = false;
};
