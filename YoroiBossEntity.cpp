#include "YoroiBossEntity.h"
#include "TransformComponent.h"
#include "CollisionComponent.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "Renderer.h"
#include <cmath>

YoroiBossEntity::YoroiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BossEntity(scene, pos, size)
    , m_attackTimer(1.0f)
    , m_rollDistanceLeft(0.0f)
    , m_attackStep(-1)
    , m_darkAttackCount(0)
    , m_maxDarkAttacks(0)
    , m_bulletActive(false)
    , m_dead(false)
    , m_deadTimer(0.0f)
    , m_dir(false)
{
}

bool YoroiBossEntity::Init()
{
    if (!BossEntity::Init()) return false;

    m_hp = AddComponent<HPComponent>(GetMaxHP());
    m_metsuMax = GetMaxHP() * 0.3;

    if (m_sprite)
    {
        m_sprite->LoadTextureDiv("assets/images/enemy/yoroi/yoroi.png", 4, 11);
    }

    m_anim = AddComponent<AnimationComponent>();

    if (m_anim && m_sprite)
    {
        m_anim->SetSprite(m_sprite);

        // 1. 待機 (01〜06番目)
        AnimationClip idleClip;
        idleClip.frames = { 1, 2, 3, 4, 5, 6 };
        idleClip.frameDurations = { 5.0f / 24.0f, 5.0f / 24.0f, 5.0f / 24.0f, 3.0f / 24.0f, 4.0f / 24.0f, 5.0f / 24.0f };
        idleClip.loop = true;
        m_anim->AddClip("idle", idleClip);

        // 2. ローリング (07〜19番目)
        AnimationClip rollClip;
        rollClip.frames = { 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
        rollClip.frameDurations = {
            4.0f / 24.0f, 4.0f / 24.0f, 8.0f / 24.0f, 3.0f / 24.0f, 3.0f / 24.0f, 2.0f / 24.0f,
            2.0f / 24.0f, 3.0f / 24.0f, 2.0f / 24.0f, 3.0f / 24.0f, 2.0f / 24.0f, 2.0f / 24.0f,
            3.0f / 24.0f
        };
        rollClip.loop = true;
        m_anim->AddClip("roll", rollClip);

        // 3. ダークアタック (20〜33番目)
        AnimationClip attackClip;
        attackClip.frames = { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 };
        attackClip.frameDurations = {
            3.0f / 24.0f, 1.0f / 24.0f, 2.0f / 24.0f, 1.0f / 24.0f, 2.0f / 24.0f, 1.0f / 24.0f,
            5.0f / 24.0f, 3.0f / 24.0f, 2.0f / 24.0f, 3.0f / 24.0f, 17.0f / 24.0f, 3.0f / 24.0f,
            2.0f / 24.0f, 1.0f / 24.0f
        };
        attackClip.loop = false;
        m_anim->AddClip("attack", attackClip);

        // 4. やられ (34〜41番目)
        AnimationClip deadClip;
        deadClip.frames = { 34, 35, 36, 37, 38, 39, 40, 41 };
        deadClip.frameDurations = {
            1.0f / 24.0f, 4.0f / 24.0f, 6.0f / 24.0f, 5.0f / 24.0f, 8.0f / 24.0f, 6.0f / 24.0f,
            2.0f / 24.0f, 2.0f / 24.0f
        };
        deadClip.loop = false;
        m_anim->AddClip("dead", deadClip);

        m_anim->Play("idle");
    }

    return true;
}

void YoroiBossEntity::Update(float deltaTime)
{
    BossEntity::Update(deltaTime);

    if (m_sprite)
    {
        m_sprite->SetSize(1.0f, 1.0f);
        GetTransform()->SetScale({ 1.0f, 1.0f });
        m_sprite->SetDrawOffset(0.0f, -55.0f);
        m_sprite->SetFlipX(m_dir);
    }
}

void YoroiBossEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    if (m_dead) return;

    if (m_hp)
    {
      
       m_hp->Damage(damage);

        // コンポーネントの残りHPが0以下になったら死亡処理へ
        if (m_hp->GetHP() <= 0)
        {
            m_dead = true;
            m_attackTimer = 0.0f;
            SetVel({ 0.0f, 0.0f });
            m_deadTimer = 0.0f;
            m_bulletActive = false;

            if (m_anim)
            {
                m_anim->Play("dead");
            }
        }
    }
}



void YoroiBossEntity::UpdateAI(float deltaTime)
{
    if (m_dead)
    {
        m_deadTimer += deltaTime;
        constexpr float DEAD_SHOW_TIME = 2.0f;
        if (m_deadTimer >= DEAD_SHOW_TIME)
        {
            SetState(Actor::State::Dead);
        }
        return;
    }

    // ---------------------------------------------------
    // ③ 左右反転フラグの更新（ここでは計算のみ行い、適用はUpdate関数で行う）
    // ---------------------------------------------------
    auto* player = static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (player)
    {
        const float distanceX = player->GetPos().x - GetPos().x;

        // ローリング突進中以外は、常にプレイヤーの方向を向くフラグを立てる
        if (m_rollDistanceLeft <= 0.0f)
        {
            m_dir = (distanceX >= 0.0f);
        }
    }

    if (m_attackTimer > 0.0f)
    {
        m_attackTimer -= deltaTime;
        return;
    }

    if (!player) return;
    const float distanceX = player->GetPos().x - GetPos().x;
    const float dir = (distanceX > 0.0f) ? 1.0f : -1.0f;

    if (fabsf(GetVel().x) > 0.0f)
        return;

    if (m_attackStep == -1)
    {
        m_attackStep = rand() % 2;
    }

    // ローリング：予備動作終了からの突進開始
    if (m_attackStep == 2)
    {
        SetVel({ dir * 500.0f, GetVel().y });
        m_rollDistanceLeft = 500.0f;
        m_attackStep = 1;
        return;
    }

    // ローリング：予備動作の開始
    if (m_attackStep == 0)
    {
        m_darkAttackCount = 0;

        if (m_anim) m_anim->Play("roll");

        // 22フレーム分のディレイを設定
        m_attackTimer = 22.0f / 24.0f;
        m_attackStep = 2;
        return;
    }
    // ダークアタック
    else
    {
        if (m_darkAttackCount == 0)
        {
            m_maxDarkAttacks = (rand() % 2) + 1;
        }

        if (m_anim) m_anim->Play("attack");

        // ★ ボスの向き（m_dir）を基準にする (trueなら右向き=1.0f、falseなら左向き=-1.0f)
        float dirSign = m_dir ? 1.0f : -1.0f;

        // ★ 初期位置をボスの向きに合わせて調整（左向きなら左側から出るように）
        Vector2d pos = GetPos();
        m_bulletPos = { pos.x + (dirSign * 32.0f), pos.y - 32.0f };

        // ★ スピードの向きをボスの向きに合わせる
        const float bulletSpeed = 500.0f;
        m_bulletVel = { dirSign * bulletSpeed, 0.0f };

        m_bulletActive = true;
        m_darkAttackCount++;

        if (m_darkAttackCount >= m_maxDarkAttacks)
        {
            m_attackStep = 0;
        }
        m_attackTimer = 3.0f;
    }
}

void YoroiBossEntity::UpdateAttack(float deltaTime)
{
    if (m_dead) return;

    auto* player = static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (m_rollDistanceLeft > 0.0f)
    {
        float moveStep = fabsf(GetVel().x) * deltaTime;
        m_rollDistanceLeft -= moveStep;

        if (m_rollDistanceLeft <= 0.0f)
        {
            m_rollDistanceLeft = 0.0f;
            SetVel({ 0.0f, GetVel().y });
            m_attackTimer = 3.0f;

            if (m_anim) m_anim->Play("idle");
        }

        if (player && !player->IsDead())
        {
            Vector2d myPos = GetPos();
            Vector2d playerPos = player->GetPos();

            float dx = playerPos.x - myPos.x;
            float dy = playerPos.y - myPos.y;
            float distSq = dx * dx + dy * dy;

            constexpr float ROLL_HIT_RADIUS = 80.0f;

            if (distSq < ROLL_HIT_RADIUS * ROLL_HIT_RADIUS)
            {
                float knockbackX = (dx < 0.0f) ? -400.0f : 400.0f;
                player->TakeDamage(15, Vector2d(knockbackX, -250.0f));
            }
        }
    }

    if (m_bulletActive)
    {
        m_bulletPos += m_bulletVel * deltaTime;
        constexpr float limit = 10000.0f;

        if (m_bulletPos.x < -100.0f || m_bulletPos.x > limit)
        {
            m_bulletActive = false;
        }

        if (m_bulletActive && player && !player->IsDead())
        {
            Vector2d playerPos = player->GetPos();

            // 縦長長方形（AABB）の当たり判定
            float dx = fabsf(playerPos.x - m_bulletPos.x);
            float dy = fabsf(playerPos.y - m_bulletPos.y);

            // 判定の広さ（弾のサイズ 30x90 と、プレイヤーのサイズ（仮に40x60）の合算値）
            // 目安として、X方向は35ピクセル以内、Y方向は75ピクセル以内なら衝突
            constexpr float HIT_BOX_X = 35.0f;
            constexpr float HIT_BOX_Y = 75.0f;

            if (dx < HIT_BOX_X && dy < HIT_BOX_Y)
            {
                float knockbackX = (playerPos.x < m_bulletPos.x) ? -300.0f : 300.0f;
                player->TakeDamage(20, Vector2d(knockbackX, -200.0f));
                m_bulletActive = false;
            }
        }
    }
}

void YoroiBossEntity::CheckCollision()
{
    if (m_dead) return;

    auto* player = static_cast<PlayScene*>(GetScene())->GetPlayer();
    if (!player || player->IsDead()) return;

    if (m_bulletActive)
    {
        // こちらも同様に縦長矩形判定に差し替え
        float dx = fabsf(player->GetPos().x - m_bulletPos.x);
        float dy = fabsf(player->GetPos().y - m_bulletPos.y);

        constexpr float HIT_BOX_X = 35.0f;
        constexpr float HIT_BOX_Y = 75.0f;

        if (dx < HIT_BOX_X && dy < HIT_BOX_Y)
        {
            float knockbackX = (player->GetPos().x < m_bulletPos.x) ? -300.0f : 300.0f;
            player->TakeDamage(20, Vector2d(knockbackX, -200.0f));
            m_bulletActive = false;
        }
    }

    if (m_rollDistanceLeft > 0.0f)
    {
        CollisionComponent* myCol = GetComponent<CollisionComponent>();
        CollisionComponent* pCol = player->GetComponent<CollisionComponent>();

        bool isHit = false;
        if (myCol && pCol)
        {
            isHit = myCol->CheckCollision(pCol);
        }
        else
        {
            float dist = (player->GetPos() - GetPos()).length();
            if (dist < 64.0f) isHit = true;
        }

        if (isHit)
        {
            float knockbackX = (player->GetPos().x < GetPos().x) ? -400.0f : 400.0f;
            player->TakeDamage(30, Vector2d(knockbackX, -300.0f));
        }
    }
}

//ダークアタック描画用
/*void YoroiBossEntity::Draw()
{
    // 親クラスやスプライトコンポーネントの通常の描画を最初に行う
    // (もし親クラスの仕様が「Draw」ではなく「Render」などなら名前に合わせてください)
    BossEntity::Draw();

    // ダークアタックがアクティブなら、紫の縦長長方形を描画
    if (m_bulletActive)
    {
        // 斬撃のサイズ（幅30ピクセル、高さ90ピクセルの縦長）
        constexpr float BULLET_W = 30.0f;
        constexpr float BULLET_H = 90.0f;

        // m_bulletPos を中心とした四隅の座標を計算
        int left = static_cast<int>(m_bulletPos.x - BULLET_W / 2.0f);
        int top = static_cast<int>(m_bulletPos.y - BULLET_H / 2.0f);
        int right = static_cast<int>(m_bulletPos.x + BULLET_W / 2.0f);
        int bottom = static_cast<int>(m_bulletPos.y + BULLET_H / 2.0f);

        // DxLibのDrawBoxで紫色（R:200, G:0, B:255）で塗りつぶす
        DrawBox(left, top, right, bottom, GetColor(200, 0, 255), TRUE);
    }
}*/