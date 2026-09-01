#include "SekienkiBossEntity.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "Renderer.h"
#include <cmath>

SekienkiBossEntity::SekienkiBossEntity(
    Scene* scene,
    const Vector2d& pos,
    const Vector2d& size)
    : BossEntity(scene, pos, size)

    , m_attackAnimFrame(0)
    , m_attackAnimTimer(0.0f)
    , m_attackStep(0)
    , m_phase(0)
    , m_attackTimer(1.0f)
    , m_jumpAttackCooldown(0.0f)
    , m_rollDistanceLeft(0.0f)
    , m_rollDir(1.0f)
    , m_rollWaitTimer(0.0f)
    , m_fire(false)
    , m_bulletActive(false)
    , m_fallStartY(0.0f)
    , m_hoverY(0.0f)
    , m_jumpOffsetApplied(false)
    , m_secondJump(false)
    , m_darkAttackFire(false)
    , m_tornadoDistanceLeft(0.0f)
    , m_dead(false)
{
}

bool SekienkiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    m_hp = AddComponent<HPComponent>(GetMaxHP());
    m_metsuMax = GetMaxHP() * 0.3;

    // コンポーネントの取得
    m_anim = AddComponent<AnimationComponent>();

    if (m_sprite)
    {
        // 3枚の画像の全フレームのハンドルを格納するベクター
        std::vector<int> allFrames;

        // DxLibの関数を直接使って、各画像を分割読み込みして1つにまとめるラムダ関数
        auto loadAndAppend = [&](const std::string& path, int xNum, int yNum) {
            int total = xNum * yNum;
            int tempHandle = LoadGraph(path.c_str());
            if (tempHandle == -1) return false;

            int texW = 0, texH = 0;
            GetGraphSize(tempHandle, &texW, &texH);
            DeleteGraph(tempHandle);

            int frameW = texW / xNum;
            int frameH = texH / yNum;

            std::vector<int> tempBuf(total);
            LoadDivGraph(path.c_str(), total, xNum, yNum, frameW, frameH, tempBuf.data());

            // 全体配列の末尾に追加
            allFrames.insert(allFrames.end(), tempBuf.begin(), tempBuf.end());
            return true;
            };

        // 3枚の画像を順番に読み込んで allFrames に結合
        loadAndAppend("assets/images/enemy/sekienki/Kozaru_1.png", 4, 7); 
        loadAndAppend("assets/images/enemy/sekienki/Kozaru_2.png", 4, 6);  
        loadAndAppend("assets/images/enemy/sekienki/Kozaru_3.png", 4, 7);   

        m_sprite->SetEffectFrames(allFrames);

        // 最初の画像の1コマ分のサイズをセット（SetSizeもpublicなのでボスから呼べます）
        m_sprite->SetSize(1.0f, 1.0f);
        GetTransform()->SetScale({ 1.0f,1.0f });
        m_sprite->SetDrawOffset(0.0f, -130.0f);
    }

    // ★重要：アニメーションクリップをここで完全に登録する
    if (m_anim && m_sprite)
    {
        m_anim->SetSprite(m_sprite);

        // 1. 待機
        AnimationClip idleClip;
        idleClip.frames = { 0, 1, 2, 3, 4, 5 };
        idleClip.frameDurations = 
        {
          3.0f / 24.0f, 
          5.0f / 24.0f, 
          2.0f / 24.0f, 
          3.0f / 24.0f, 
          3.0f / 24.0f, 
          5.0f / 24.0f 
        };
        idleClip.loop = true;
        m_anim->AddClip("idle", idleClip);

        // 2. 近接攻撃(火炎)
        AnimationClip attackClip;
        attackClip.frames = { 7, 20, 21, 22, 23, 24, 25, 26, 27, 26, 7 };
        attackClip.frameDurations = 
        { 
          2.0f / 24.0f, 
          3.0f / 24.0f, 
          6.0f / 24.0f,
          8.0f / 24.0f,
          5.0f / 24.0f, 
          2.0f / 24.0f,
          1.0f / 24.0f,
          1.0f / 24.0f,
          16.0f / 24.0f,
          1.0f / 24.0f,
          2.0f / 24.0f,
        };
        attackClip.loop = false;
        m_anim->AddClip("attack", attackClip);

        // 3. ローリング
        AnimationClip rollClip;
        rollClip.frames = { 7, 8, 9, 10, 11, 12, 13, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        rollClip.frameDurations =
        {
          2.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          1.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          1.0f / 24.0f,
        };
        rollClip.loop = false;
        m_anim->AddClip("roll", rollClip);

        // 4. ドラミング(形態変化)
        AnimationClip drummingClip;
        drummingClip.frames = { 13, 8, 9, 10, 11, 12, 13, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 8, 9, 10, 11, 12 };
        drummingClip.frameDurations = 
        {
          2.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
        };
        drummingClip.loop = false;
        m_anim->AddClip("drumming", drummingClip);

        // 5. 落下攻撃1
        AnimationClip fallClip;
        fallClip.frames = 
        { 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 44, 42, 43, 47, 48, 49, 21, 22, 23, 22 };
        fallClip.frameDurations =
        {
          3.0f / 24.0f, 
          5.0f / 24.0f,
          6.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          1.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          24.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          2.0f / 24.0f,
          3.0f / 24.0f,
          1.0f / 24.0f,
          80.0f / 24.0f, //着地まで
          4.0f / 24.0f,
          4.0f / 24.0f,
          3.0f / 24.0f,
          3.0f / 24.0f,
        };
        fallClip.loop = false;
        m_anim->AddClip("fall", fallClip);

        // 6. 落下攻撃2
        AnimationClip fall2Clip;
        fall2Clip.frames = {
            26, 38, 39, 40, 41, 42, 43, 44, 45, 46, 44, 42, 43, 47, 48, 49, 21, 22, 23, 22
        };
        fall2Clip.frameDurations = {
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            3.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            3.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            4.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            3.0f / 24.0f,
            1.0f / 24.0f,
            50.0f / 24.0f,
            4.0f / 24.0f,
            4.0f / 24.0f,
            3.0f / 24.0f,
            3.0f / 24.0f,
        };
        fall2Clip.loop = false;
        m_anim->AddClip("fall2", fall2Clip);

        // 7. ダークアタック
        AnimationClip darkAttackClip;
        darkAttackClip.frames = {
            52, 53, 54, 55, 54, 56, 57, 58, 57, 56, 57, 58, 57, 56, 57, 58, 57, 56, 57,
            58, 57, 56, 57, 58, 59, 60, 61, 62, 63
        };
        darkAttackClip.frameDurations ={
            4.0f / 24.0f,
            4.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            2.0f / 24.0f,
            1.0f / 24.0f,
            1.0f / 24.0f,
            5.0f / 24.0f,
            4.0f / 24.0f,
        };
        darkAttackClip.loop = false;
        m_anim->AddClip("dark_attack", darkAttackClip);

        // 8. 死亡
        AnimationClip deadClip;
        deadClip.frames = {
            64, 65, 66, 67, 68, 69,70, 71, 72, 73, 74, 75, 76
        };
        deadClip.frameDurations = {
            7.0f / 24.0f,
            3.0f / 24.0f,
            8.0f / 24.0f,
            8.0f / 24.0f,
            13.0f / 24.0f,
            6.0f / 24.0f,
            6.0f / 24.0f,
            4.0f / 24.0f,
            4.0f / 24.0f,
            6.0f / 24.0f,
            6.0f / 24.0f,
            4.0f / 24.0f,
            6.0f / 24.0f,
        };
        deadClip.loop = false;
        m_anim->AddClip("dead", deadClip);

        // 初期再生
        m_anim->Play("idle");
    }
    return true;
}

void SekienkiBossEntity::Update(float deltaTime)
{
    BossEntity::Update(deltaTime);

    if(m_sprite)
    {
        if (m_attackStep == 1 || m_attackStep == 8)
        {
            // ローリング中だけ向きを固定
            m_sprite->SetFlipX(m_rollDir > 0.0f);
        }
        else
        {
            auto* player =
                static_cast<PlayScene*>(GetScene())->GetPlayer();

            if (player)
            {
                float dir =
                    player->GetPos().x > GetPos().x ?
                    1.0f : -1.0f;

                m_sprite->SetFlipX(dir > 0.0f);
            }
        }
    }
}

//---------------------------------
// 飛び上がり攻撃管理
//---------------------------------

void SekienkiBossEntity::StartJumpAttack()
{
    if (m_anim)
    {
        if (m_secondJump)
            m_anim->Play("fall2", false);
        else
            m_anim->Play("fall", false);

        m_attackAnimFrame = 0;
        m_attackAnimTimer = 0.0f;

        m_attackStep = 3;

        m_attackTimer = 1.5f;

        SetVel({ 0.0f,0.0f });

        Vector2d pos = GetPos();

        SetPos(pos);

        m_hoverY = pos.y;
        m_jumpOffsetApplied = false;
    }
}

    //---------------------------------
    // 形態移行管理
    //---------------------------------

    void SekienkiBossEntity::PhaseChange()
    {
        float hpRate =
            (float)m_hp->GetHP() / GetMaxHP();

        if (m_phase < 2 && hpRate <= 0.5f && m_attackStep == 0)
        {
            m_phase = 2;

            m_attackStep = 12;

            if (m_anim)
                m_anim->Play("drumming", false);

            m_attackAnimFrame = 0;
            m_attackAnimTimer = 0.0f;

            m_attackTimer = 2.0f;

            SetVel({ 0.0f,0.0f });

            m_hp->SetInvincible(9999.0f);
        }
        else if (m_phase < 1 && hpRate <= 0.7f && m_attackStep == 0)
        {
            m_phase = 1;

            m_attackStep = 11;

            if (m_anim)
                m_anim->Play("drumming", false);

            m_attackAnimFrame = 0;
            m_attackAnimTimer = 0.0f;

            m_attackTimer = 2.0f;

            SetVel({ 0.0f,0.0f });

            m_hp->SetInvincible(9999.0f);
        }
    }



void SekienkiBossEntity::UpdateAI(float deltaTime)
{
    if (m_dead) { 
        SetState(Actor::State::Dead);
        return; 
    }

    PhaseChange();
    if (m_attackStep == 11 || m_attackStep == 12)
    {
        return;
    }


    auto* player =
        static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (!player)
        return;

    if (m_attackStep != 0)
        return;

    if (m_attackTimer > 0.0f)
        return;

    const float distanceX =
        player->GetPos().x - GetPos().x;

    const float absDistance =
        fabsf(distanceX);

    const float dir =
        distanceX > 0.0f ? 1.0f : -1.0f;

    const float farDistance = 150.0f;


    //---------------------------------
    // 行動選択
    //---------------------------------

    int r = rand() % 100;

    float hp70 = GetMaxHP() * 0.7f;
    float hp50 = GetMaxHP() * 0.5f;

    //---------------------------------
    // HP50%
    //---------------------------------
    if (m_hp->GetHP() <= hp50)
    {
        if (r < 50)
        {
            m_attackStep = 8;

            if (m_anim)
                m_anim->Play("roll", false);

            m_attackAnimFrame = 0;
            m_attackAnimTimer = 0.0f;

            m_rollWaitTimer = 30.0f / 24.0f;

            // プレイヤーと反対方向へローリング
            m_rollDir = -dir;

            SetVel({ 0.0f, GetVel().y });

            m_rollDistanceLeft = 500.0f;

            return;
        }
        else if (r < 90)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }

    //---------------------------------
    // HP70%
    //---------------------------------
    else if (m_hp->GetHP() <= hp70)
    {
        if (r < 40)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }

    //---------------------------------
    // 基本行動
    //---------------------------------
    if (absDistance >= farDistance)
    {
        m_attackStep = 1;
    }
    else
    {
        m_attackStep = (r < 80) ? 2 : 1;
    }

    //---------------------------------
    // 前転攻撃
    //---------------------------------

    if (m_attackStep == 1)
    {
        m_anim->Play("roll", false);
        m_attackAnimFrame = 0;
        m_attackAnimTimer = 0.0f;

        m_rollWaitTimer = 30.0f / 24.0f;    // 約30F待つ

        // ★ローリング開始時の向きを保存
        m_rollDir = dir;

        SetVel({ 0.0f, GetVel().y });

        m_rollDistanceLeft = 500.0f;
    }

    //---------------------------------
    // 近接攻撃
    //---------------------------------

    else if (m_attackStep == 2)
    {
        if (m_anim)
            m_anim->Play("attack", false);
        m_attackAnimFrame = 0;
        m_attackAnimTimer = 0.0f;
        m_fire = false;

        m_attackTimer = 10.0f;
    }

    //---------------------------------
    // 飛び上がり
    //---------------------------------

    if (m_jumpAttackCooldown <= 0.0f)
    {

        if (m_hp->GetHP() > hp70)
            return;

        if (rand() % 100 < 40)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }
}

void SekienkiBossEntity::UpdateAttack(float deltaTime)
{

    if (m_dead) return;

    int frame = m_sprite->GetCurrentFrame();

    auto* player =
        static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (!player)
        return;

    float dir =
        player->GetPos().x > GetPos().x ?
        1.0f : -1.0f;

    //---------------------------------
    // タイマー
    //---------------------------------

    if (m_attackTimer > 0.0f)
    {
        m_attackTimer -= deltaTime;
    }

    //---------------------------------
    // 近接攻撃
    //---------------------------------

    if (!m_fire &&
        frame == 26)
    {
        m_fire = true;
        m_bulletActive = true;

        // 弾生成
        if (dir > 0.0f)
        {
            m_bulletPos =
            {
                GetPos().x + 64.0f,
                GetPos().y - 55.0f
            };

            m_bulletVel =
            {
                300.0f,
                0.0f
            };
        }
        else
        {
            m_bulletPos =
            {
                GetPos().x - 80.0f,
                GetPos().y - 55.0f
            };

            m_bulletVel =
            {
                -300.0f,
                0.0f
            };
        }
    }

    //---------------------------------
    // 近接攻撃終了
    //---------------------------------

    if (m_attackStep == 2 &&
        m_anim->IsFinished())
    {
        m_bulletActive = false;

        m_attackStep = 0;

        m_attackTimer = 2.0f;

        m_anim->Play("idle");
    }

    //---------------------------------
    // 前転攻撃
    //---------------------------------

    if (m_attackStep == 1)
    {
        // 溜めモーション
        if (m_rollWaitTimer > 0.0f)
        {
            m_rollWaitTimer -= deltaTime;
            SetVel({ 0.0f, GetVel().y });
        }
        else
        {
            SetVel({ m_rollDir * 900.0f, GetVel().y });
            TryDamagePlayer();

            float moveStep = fabsf(GetVel().x) * deltaTime;
            m_rollDistanceLeft -= moveStep;

            if (m_rollDistanceLeft <= 0.0f)
            {
                m_rollDistanceLeft = 0.0f;

                SetVel({ 0.0f, GetVel().y });

                m_attackStep = 0;
                m_attackTimer = 3.0f;

                if (m_anim)
                    m_anim->Play("idle");
            }
        }
    }

    //---------------------------------
    // 飛び上がり
    //---------------------------------

    if (m_attackStep == 3)
    {
        SetVel({ 0.0f, 0.0f });
        SetPos({ GetPos().x, m_hoverY });

        // 44Fで一度だけ220px上昇
        if (frame >= 44 && !m_jumpOffsetApplied)
        {
            Vector2d pos = GetPos();
            pos.y -= 250.0f;
            SetPos(pos);

            // 基準座標も更新する
            m_hoverY = pos.y;

            m_jumpOffsetApplied = true;

            m_sprite->SetDrawOffset(0.0f, 115.0f);
        }

        // 49Fで画面外へワープ
        if (frame >= 49)
        {
            m_attackStep = 4;
            m_attackTimer = 3.0f;

            SetPos({ GetPos().x, -150.0f });
            m_sprite->SetDrawOffset(0.0f, -130.0f);
        }
    }

    //---------------------------------
    //空中追尾
    //---------------------------------

    if (m_attackStep == 4)
    {
        auto* player =
            static_cast<PlayScene*>(GetScene())->GetPlayer();

        if (player)
        {
            float dx =
                (player->GetPos().x + 16.0f) -
                (GetPos().x + 32.0f);

            SetVel(
                {
                    dx > 0.0f ?
                    750.0f :
                    -750.0f,
                    0.0f
                });
        }

        if (m_attackTimer <= 1.0f)
        {
            SetVel({ 0.0f,0.0f });
        }

        if (m_attackTimer <= 0.0f)
        {
            m_attackStep = 5;

            m_fallStartY = GetPos().y;
        }
    }

    //---------------------------------
    // 落下攻撃
    //---------------------------------

    if (m_attackStep == 5)
    {
        TryDamagePlayer();
        SetVel({ 0.0f, 1000.0f });

        if (m_isGround)
        {
            m_attackStep = 6;
            m_attackTimer = 0.6f;
        }
    }

    //---------------------------------
    // 落下攻撃終了＆連続飛び上がり
    //---------------------------------

    if (m_attackStep == 6 &&
        m_attackTimer <= 0.0f)
    {
        if (m_phase == 2 && !m_secondJump)
        {
            m_attackStep = 7;

            m_attackTimer = 1.0f;

            SetVel({ 0.0f, 0.0f });

        }
        else
        {
            m_attackStep = 0;

            m_attackTimer = 1.0f;

            if (m_anim)
                m_anim->Play("idle");

            m_jumpAttackCooldown = 8.0f;

        }
    }

    if (m_attackStep == 7 &&
        m_attackTimer <= 0.0f)
    {
        m_secondJump = true;
        StartJumpAttack();
    }

    //---------------------------------
    // 前転攻撃（ダークアタック直前）
    //---------------------------------

    if (m_attackStep == 8)
    {

        if (m_rollWaitTimer > 0.0f)
        {
            m_rollWaitTimer -= deltaTime;
            SetVel({ 0.0f, GetVel().y });
        }
        else
        {
            SetVel({ m_rollDir * 900.0f, GetVel().y });
            TryDamagePlayer();

            float moveStep = fabsf(GetVel().x) * deltaTime;
            m_rollDistanceLeft -= moveStep;

            if (m_rollDistanceLeft <= 0.0f)
            {
                m_rollDistanceLeft = 0.0f;

                SetVel({ 0.0f, GetVel().y });

                m_attackStep = 9;
                m_attackTimer = 3.0f;

                m_darkAttackFire = false;

                if (m_anim)
                {
                    m_anim->Play("dark_attack", false);
                }

                m_attackAnimFrame = 0;
                m_attackAnimTimer = 0.0f;
                m_darkAttackFire = false;
            }
        }
    } 

    //---------------------------------
    // ダークアタック
    //---------------------------------

    if (m_attackStep == 9 && !m_darkAttackFire && frame == 63)   // 後で発射位置を調整
    {
        m_darkAttackFire = true;

        m_bulletActive = true;

        m_bulletPos =
        {
            GetPos().x,
            GetPos().y - 120.0f
        };

        if (player)
        {
            float dir =
                player->GetPos().x > GetPos().x ?
                1.0f : -1.0f;

            m_bulletVel =
            {
                dir * 300.0f,
                0.0f
            };
        }

        m_tornadoDistanceLeft = 600.0f;
    }

    if (m_attackStep == 9 &&
        m_anim->IsFinished())
    {
        m_attackStep = 10;
    }

    if (m_attackStep == 10 &&
        m_bulletActive)
    {
        m_bulletPos.x +=
            m_bulletVel.x * deltaTime;

        float move =
            fabsf(m_bulletVel.x) * deltaTime;

        m_tornadoDistanceLeft -= move;

        if (m_tornadoDistanceLeft <= 0.0f)
        {
            m_bulletActive = false;

            m_attackStep = 0;

            m_attackTimer = 1.0f;

            m_jumpAttackCooldown = 7.0f;

            m_anim->Play("idle");
        }
    }

    //---------------------------------
    // 形態移行管理
    //---------------------------------

    if (m_attackStep == 11)
    {
        SetVel({ 0.0f, 0.0f });

        if (m_anim->IsFinished())
        {
            m_hp->SetInvincible(0.0f);

            m_attackStep = 0;
            m_attackTimer = 1.0f;

            m_anim->Play("idle");
        }

        return;
    }

    if (m_attackStep == 12)
    {
        SetVel({ 0.0f, 0.0f });

        if (m_anim->IsFinished())
        {
            m_hp->SetInvincible(0.0f);

            m_attackStep = 0;
            m_attackTimer = 1.0f;

            m_anim->Play("idle");
        }

        return;
    }   

    //---------------------------------
    // 死亡
    //---------------------------------

    if (m_attackStep == 13)
    {
        if (m_anim->IsFinished())
        {
            // 撃破処理
            m_dead = true;
        }

        return;
    }

}

void SekienkiBossEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    // すでに死亡しているなら何もしない
    if (m_dead) return;

    if (m_hp)
    {
        // HPを減らす
        m_hp->Damage(damage);

        // HPが0以下になったら死亡処理へ
        if (m_hp->GetHP() <= 0)
        {
            m_attackStep = 13;       // 攻撃ステートをリセット
            m_attackTimer = 0.0f;   // タイマー停止
            SetVel({ 0.0f, 0.0f }); // その場に停止
            m_bulletActive = false; // 攻撃用の弾（火炎など）を強制非アクティブ化

            // 死亡アニメーションを再生
            if (m_anim)
            {
                m_anim->Play("dead");
            }
        }
    }
}

bool SekienkiBossEntity::TryDamagePlayer()
{
    for (Actor* actor : GetScene()->GetActors())
    {
        if (actor == nullptr ||
            actor->GetType() != ActorType::Player ||
            actor->IsDead())
        {
            continue;
        }

        auto* playerCollision =
            actor->GetComponent<CollisionComponent>();

        if (!playerCollision)
            continue;

        if (!m_collision->CheckCollision(playerCollision))
            continue;

        auto* player =
            static_cast<PlayerEntity*>(actor);

        // ノックバック方向
        float knockbackX =
            GetPos().x < player->GetPos().x ?
            500.0f : -500.0f;

        player->TakeDamage(
            20,                         // ダメージ量
            Vector2d(knockbackX, 0.0f)  // ノックバック
        );

        return true;
    }

    return false;
}