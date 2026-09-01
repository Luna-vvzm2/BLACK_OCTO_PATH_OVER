#include "HPComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "PlayScene.h"
#include <algorithm>    // std::clamp
#include <iostream>

HPComponent::HPComponent(Actor* owner, int maxHP)
    : Component(owner)
    , m_owner(owner)
    , m_maxHP(maxHP)
    , m_hp(maxHP)
    , m_invincibleTimer(0.0f)
{
}

void HPComponent::Update(float deltaTime)
{
    if (m_invincibleTimer > 0.0f) {
        m_invincibleTimer -= deltaTime;
        if (m_invincibleTimer < 0.0f)
            m_invincibleTimer = 0.0f;
    }
}

void HPComponent::Damage(int amount)
{
    std::cout << "Damage 呼ばれた: " << amount << std::endl;

    int old = m_hp;
    m_hp = std::clamp(m_hp - amount, 0, m_maxHP);

    if (OnHPChanged) OnHPChanged(m_hp, old);

    // 敵（ActorType::Enemy）がダメージを受けた場合、PlayScene のコンボを加算する
    // ※ Attack の発生源を渡していないため「any による敵ダメージ」をコンボに数えます。
    if (m_owner) {
        try {
            if (m_owner->GetType() == ActorType::Enemy) {
                Scene* scene = m_owner->GetScene();
                if (scene && scene->GetType() == Scene::Type::Play) {
                    PlayScene* play = static_cast<PlayScene*>(scene);
                    play->AddCombo();
                }
            }
        }
        catch (...) {
            // 安全のため例外は握り潰す（通常は発生しない）
        }
    }

    if (m_hp == 0) {
        if (OnDeath) OnDeath();
        //m_owner->SetState(Actor::State::Dead); // 元の挙動に従う
    }
}

void HPComponent::Heal(int amount)
{
    int old = m_hp;
    m_hp = std::clamp(m_hp + amount, 0, m_maxHP);

    if (OnHPChanged) OnHPChanged(m_hp, old);
}
