#include "PlayerEntity.h"
#include "EnemyEntity.h"
#include "EffectActor.h"
#include "Scene.h"
#include "PlayScene.h"
#include "BlockActor.h"
#include "KunaiActor.h"
#include "TreasureBox.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "KaryuTextUI.h"
#include "Input.h"
#include "Game.h"
#include <Windows.h>

//                       offset, width, height, damage, metsu
AttackHitbox weak1{ Vector2d(100,0), 300, 200, 70, 15 };
AttackHitbox weak2{ Vector2d(100,0), 300, 200, 70, 15 };
AttackHitbox weak3{ Vector2d(100,0), 300, 200, 70, 15 };
AttackHitbox weak4{ Vector2d(100,0), 300, 200, 70, 15 };
AttackHitbox strong1{ Vector2d(100,-50), 400, 300, 150, 60 };
AttackHitbox strong2{ Vector2d(100,-50), 400, 300, 170, 60 };
AttackHitbox airWeak1{ Vector2d(100,50), 300, 200, 70, 15 };
AttackHitbox airWeak2{ Vector2d(100,50), 300, 200, 70, 15 };
AttackHitbox airWeak3{ Vector2d(100,50), 300, 200, 70, 15 };
AttackHitbox Hayabusa{ Vector2d(50,100), 100, 100, 150, 60 };
AttackHitbox squatAttack{ Vector2d(0,0), 450, 100, 70, 15 };


PlayerEntity::PlayerEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_hp(nullptr)
    , m_hpMax(100)
    , m_gravity(nullptr)
    , m_anim(nullptr)

    , m_combo(0)
    , m_kunai(10)

    , m_state(PlayerState::IDLE)

    , m_kunaiSpawnTimer(0.0f)
    , m_kunaiPending(false)

    , m_dir(true)
    , m_prevDir(true)
    , m_jumpSpeed(0.0f)
    , m_moveSpeed(290.0f)
    , m_dashSpeed(600.0f)
    , m_dashAirSpeed(800.0f)
    , m_dashTimer(0.0f)
    , m_HienCount(0)
    , m_isSenten(false)

    , m_jumpCount(0)
    , m_jumpTime(0.0f)
    , m_maxJumpTime(1.5f)
    , m_attack(false)
    , m_hit(false)
    , m_HayabusaHit(false)
    , m_attackType()
    , m_attackCol(nullptr)
    , m_weakAttackIdx(0)
    , m_strongAttackIdx(0)
    , m_airAttackIdx(0)
    , m_attackTimer(0.0f)

    , m_canAttack(true)
    , m_attackLockTimer(0.0f)

    , m_getHit(false)
    , m_getHitTimer(0.0f)
    , m_invincibleTime(0.0f)

    , m_canMove(true)
    , m_squat(false)
    , m_canStand(true)
    , m_canCharge(true)

    , m_jutsuGauge(0)
    , m_jutsuCharge(false)

    , m_isJutsuKamae(false)
    , m_isKaryu(false)
    , m_KaryuTimer(0.0f)

    , m_isKamae(false)
    , m_isExecution(false)
    , m_executionTimer(0.0f)

    , m_coin(0)
    , m_haku(0)
    , m_maxHaku(100)

    , m_ignorePlatform(false)

    //Sound

    , m_kunaiSound(nullptr)
    , m_excusionSound(nullptr)
    , m_kamaeSound(nullptr)
    , m_karyuSound(nullptr)
    , m_weakAttackSound1(nullptr)
    , m_weakAttackSound2(nullptr)
    , m_weakAttackSound3(nullptr)
    , m_weakAttackSound4(nullptr)
    , m_strongAttackSound1(nullptr)
    , m_strongAttackSound2(nullptr)
    , m_hayabusaAttackSound(nullptr)
    , m_karaburiSound(nullptr)
{
}

bool PlayerEntity::Init() {
    if (!EntityActor::Init()) return false;

    m_hp = AddComponent<HPComponent>(m_hpMax);
    m_transform->SetScale({ 1.0f, 1.0f });
    m_gravity = AddComponent<GravityComponent>(2800.0f);
    m_attackCol = AddComponent<CollisionComponent>();

    //m_sprite = AddComponent<SpriteComponent>();
    m_sprite->LoadTextureDiv("assets/images/entities/players/musashi_sheet.png", 20, 14);
    m_sprite->LoadTextureDiv("kari/player.png", 6, 4);

    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip idle;
    idle.frames = { 0, 1, 2, 3, 4, 5 };
    idle.speed = 0.3f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip runStart;
    runStart.frames = { 6, 7 };
    runStart.speed = 0.1f;
    runStart.loop = false;
    m_anim->AddClip("runStart", runStart);

    AnimationClip run;
    run.frames = { 8, 9, 10, 11, 12, 13 };
    run.speed = 0.1f;
    run.loop = true;
    m_anim->AddClip("run", run);

    AnimationClip stopShort;
    stopShort.frames = { 14, 14, 17, 17, 18, 15 };
    stopShort.speed = 0.1f;
    stopShort.loop = false;
    m_anim->AddClip("stopShort", stopShort);

    AnimationClip stopLong;
    stopLong.frames = { 16, 17, 17, 18, 15 };
    stopLong.speed = 0.1f;
    stopLong.loop = false;
    m_anim->AddClip("stopLong", stopLong);

    AnimationClip changeDir;
    changeDir.frames = { 19, 20 };
    changeDir.speed = 0.1f;
    changeDir.loop = false;
    m_anim->AddClip("changeDir", changeDir);

    AnimationClip changeDirRun;
    changeDirRun.frames = { 21, 22, 23, 24, 25, 26 };
    changeDirRun.speed = 0.1f;
    changeDirRun.loop = false;
    m_anim->AddClip("changeDirRun", changeDirRun);

    AnimationClip squatStart;
    squatStart.frames = { 27 };
    squatStart.speed = 0.1f;
    squatStart.loop = false;
    m_anim->AddClip("squatStart", squatStart);

    AnimationClip squat;
    squat.frames = { 28, 28, 28, 29 };
    squat.speed = 0.1f;
    squat.loop = false;
    m_anim->AddClip("squat", squat);

    AnimationClip squatIdle;
    squatIdle.frames = { 29 };
    squatIdle.speed = 0.1f;
    squatIdle.loop = false;
    m_anim->AddClip("squatIdle", squatIdle);

    AnimationClip squatWalk;
    squatWalk.frames = { 30, 30, 30, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33,  34, 35, 35, 35 };
    squatWalk.speed = 0.1f;
    squatWalk.loop = true;
    m_anim->AddClip("squatWalk", squatWalk);

    AnimationClip squatAttack;
    squatAttack.frames = { 37, 38, 39, 40, 40, 40, 40, 41, 41, 42, 42 };
    squatAttack.speed = 0.1f;
    squatAttack.loop = false;
    m_anim->AddClip("squatAttack", squatAttack);

    AnimationClip jumpStart;
    jumpStart.frames = { 43 };
    jumpStart.speed = 0.1f;
    jumpStart.loop = false;
    m_anim->AddClip("jumpStart", jumpStart);

    AnimationClip jump;
    jump.frames = { 44, 45, 46, 47, 48 };
    jump.speed = 0.2f;
    jump.loop = false;
    m_anim->AddClip("jump", jump);

    AnimationClip jumpSecond;
    jumpSecond.frames = { 49, 50, 51, 52, 53, 54, 55, 56, 57, 52, 53, 54, 55, 56, 58 };
    jumpSecond.speed = 0.1f;
    jumpSecond.loop = false;
    m_anim->AddClip("jumpSecond", jumpSecond);

    AnimationClip fall;
    fall.frames = { 59, 60, 61 };
    fall.speed = 0.1f;
    fall.loop = false;
    m_anim->AddClip("fall", fall);

    AnimationClip jumpLanding;
    jumpLanding.frames = { 62, 63, 64, 65, 66 };
    jumpLanding.speed = 0.1f;
    jumpLanding.loop = false;
    m_anim->AddClip("jumpLanding", jumpLanding);

    AnimationClip roll;
    roll.frames = { 67, 68, 69, 70, 71 };
    roll.speed = 0.2f;
    roll.loop = false;
    m_anim->AddClip("roll", roll);

    AnimationClip Hien;
    Hien.frames = { 72, 73, 73, 73, 73, 73, 73, 74, 74 };
    Hien.speed = 0.1f;
    Hien.loop = false;
    m_anim->AddClip("Hien", Hien);

    AnimationClip Senten;
    Senten.frames = { 75, 76, 77, 78, 79, 80, 81, 82, 83 };
    Senten.speed = 0.1f;
    Senten.loop = false;
    m_anim->AddClip("Senten", Senten);

    AnimationClip rollLanding;
    rollLanding.frames = { 84, 85, 86 };
    rollLanding.speed = 0.1f;
    rollLanding.loop = false;
    m_anim->AddClip("rollLanding", rollLanding);

    AnimationClip SouKunaiSenten;
    SouKunaiSenten.frames = { 87, 88, 89, 89, 89, 89, 90, 91, 92, 93, 94, 95 };
    SouKunaiSenten.speed = 0.1f;
    SouKunaiSenten.loop = false;
    m_anim->AddClip("SouKunaiSenten", SouKunaiSenten);

    AnimationClip wallHold;
    wallHold.frames = { 96 };
    wallHold.speed = 0.1f;
    wallHold.loop = false;
    m_anim->AddClip("wallHold", wallHold);

    AnimationClip wallJump;
    wallJump.frames = { 97, 98 };
    wallJump.speed = 0.1f;
    wallJump.loop = false;
    m_anim->AddClip("wallJump", wallJump);

    AnimationClip wallClimbUp; //　壁を歩いて登るアニメーション
    wallClimbUp.frames = { 99, 100, 101, 102 };
    wallClimbUp.speed = 0.1f;
    wallClimbUp.loop = true;
    m_anim->AddClip("wallClimbUp", wallClimbUp);

    AnimationClip wallClimb; // 壁よじ登り
    wallClimb.frames = { 103, 103, 104, 105, 105, 106 };
    wallClimb.speed = 0.1f;
    wallClimb.loop = false;
    m_anim->AddClip("wallClimb", wallClimb);

    AnimationClip platformClimb;
    platformClimb.frames = { 107, 108, 108, 109, 110, 110 };
    platformClimb.speed = 0.1f;
    platformClimb.loop = false;
    m_anim->AddClip("platformClimb", platformClimb);

    AnimationClip climbEnd;
    wallClimb.frames = { 111, 112, 113, 114, 115, 115 };
    wallClimb.speed = 0.1f;
    wallClimb.loop = false;
    m_anim->AddClip("wallClimb", wallClimb);

    AnimationClip wallAttack;
    wallAttack.frames = { 116, 116, 117, 118, 118, 119, 120, 120, 121, 121, 122, 122 };
    wallAttack.speed = 0.1f;
    wallAttack.loop = false;
    m_anim->AddClip("wallAttack", wallAttack);

    AnimationClip wallKunai;
    wallKunai.frames = { 123, 123, 123, 124, 125, 126, 127, 127, 128, 128 };
    wallKunai.speed = 0.1f;
    wallKunai.loop = false;
    m_anim->AddClip("wallKunai", wallKunai);

    AnimationClip weakAttack1;
    weakAttack1.frames = { 129, 130, 130, 131, 131, 131, 131, 131, 131, 131 };
    weakAttack1.speed = 0.1f;
    weakAttack1.loop = false;
    m_anim->AddClip("weakAttack1", weakAttack1);

    AnimationClip weakAttack2;
    weakAttack2.frames = { 132, 133, 134, 134, 135, 135, 136, 137 };
    weakAttack2.speed = 0.1f;
    weakAttack2.loop = false;
    m_anim->AddClip("weakAttack2", weakAttack2);

    AnimationClip weakAttack3;
    weakAttack3.frames = { 138, 139, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140 };
    weakAttack3.speed = 0.1f;
    weakAttack3.loop = false;
    m_anim->AddClip("weakAttack3", weakAttack3);

    AnimationClip attackEnd;
    attackEnd.frames = { 141, 142, 142, 143, 143 };
    attackEnd.speed = 0.1f;
    attackEnd.loop = false;
    m_anim->AddClip("attackEnd", attackEnd);

    AnimationClip weakAttack4;
    weakAttack4.frames = { 144, 144, 145, 146, 146, 146, 147, 147, 148, 149, 150, 150, 150, 151, 151, 152, 152, 152, 153, 153, 154, 154, 154, 155, 155, 156 };
    weakAttack4.speed = 0.1f;
    weakAttack4.loop = false;
    m_anim->AddClip("weakAttack4", weakAttack4);

    AnimationClip weakAirAttack1;
    weakAirAttack1.frames = { 157, 158, 159, 159 };
    weakAirAttack1.speed = 0.1f;
    weakAirAttack1.loop = false;
    m_anim->AddClip("weakAirAttack1", weakAirAttack1);

    AnimationClip weakAirAttack2;
    weakAirAttack2.frames = { 160, 161, 161 };
    weakAirAttack2.speed = 0.1f;
    weakAirAttack2.loop = false;
    m_anim->AddClip("weakAirAttack2", weakAirAttack2);

    AnimationClip weakAirAttack3;
    weakAirAttack3.frames = { 162, 162, 163, 164, 164, 165, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166,  167, 167, 167, 168, 168, 169, 169, 169 };
    weakAirAttack3.speed = 0.1f;
    weakAirAttack3.loop = false;
    m_anim->AddClip("weakAirAttack3", weakAirAttack3);

    AnimationClip strongAttack1;
    strongAttack1.frames = { 170, 171, 172, 172, 173, 173, 174, 175, 175, 176, 176, 177, 178, 178, 179 };
    strongAttack1.speed = 0.1f;
    strongAttack1.loop = false;
    m_anim->AddClip("strongAttack1", strongAttack1);

    AnimationClip strongAttackEnd;
    strongAttackEnd.frames = { 180, 180 };
    strongAttackEnd.speed = 0.2f;
    strongAttackEnd.loop = false;
    m_anim->AddClip("strongAttackEnd", strongAttackEnd);

    AnimationClip strongAttack2;
    strongAttack2.frames = { 181, 181, 182, 183, 183, 184, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 191, 191, 191, 191 };
    strongAttack2.speed = 0.1f;
    strongAttack2.loop = false;
    m_anim->AddClip("strongAttack2", strongAttack2);

    AnimationClip Hayabusa;
    Hayabusa.frames = { 192, 192, 193 };
    Hayabusa.speed = 0.1f;
    Hayabusa.loop = false;
    m_anim->AddClip("Hayabusa", Hayabusa);

    AnimationClip HayabusaHit;
    HayabusaHit.frames = { 194, 194, 194, 195, 195, 196, 196, 197, 197, 198 };
    HayabusaHit.speed = 0.1f;
    HayabusaHit.loop = false;
    m_anim->AddClip("HayabusaHit", HayabusaHit);

    AnimationClip HayabusaGround;
    HayabusaGround.frames = { 199 };
    HayabusaGround.speed = 0.1f;
    HayabusaGround.loop = false;
    m_anim->AddClip("HayabusaGround", HayabusaGround);

    AnimationClip Kunai;
    Kunai.frames = { 200, 201, 202, 202, 203, 204 };
    Kunai.speed = 0.1f;
    Kunai.loop = false;
    m_anim->AddClip("Kunai", Kunai);

    AnimationClip KunaiAir;
    KunaiAir.frames = { 205, 206, 207, 207 };
    KunaiAir.speed = 0.1f;
    KunaiAir.loop = false;
    m_anim->AddClip("KunaiAir", KunaiAir);

    AnimationClip KunaiSquat;
    KunaiSquat.frames = { 208, 209, 209, 210, 210, 211, 211, 212, 213, 213 };
    KunaiSquat.speed = 0.1f;
    KunaiSquat.loop = false;
    m_anim->AddClip("KunaiSquat", KunaiSquat);

    AnimationClip hitTrap;
    hitTrap.frames = { 214, 215, 216, 217, 218, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219 };
    hitTrap.speed = 0.1f;
    hitTrap.loop = false;
    m_anim->AddClip("trapHit", hitTrap);

    AnimationClip hitGround;
    hitGround.frames = { 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 221, 222, 223, 224, 225 };
    hitGround.speed = 0.1f;
    hitGround.loop = false;
    m_anim->AddClip("hitGround", hitGround);

    AnimationClip hitAir;
    hitAir.frames = { 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 227, 227, 228, 228, 229, 229, 230, 231 };
    hitAir.speed = 0.1f;
    hitAir.loop = false;
    m_anim->AddClip("hitAir", hitAir);

    AnimationClip hitAirLanding;
    hitAirLanding.frames = { 232, 233, 234, 235, 236, 237, 237, 238, 238, 239 };
    hitAirLanding.speed = 0.1f;
    hitAirLanding.loop = false;
    m_anim->AddClip("hitAirLanding", hitAirLanding);

    AnimationClip dead;
    dead.frames = { 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,  241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246 };
    dead.speed = 0.1f;
    dead.loop = false;
    m_anim->AddClip("dead", dead);

    AnimationClip Kamae;
    Kamae.frames = { 247, 248, 248, 249 };
    Kamae.speed = 0.1f;
    Kamae.loop = false;
    m_anim->AddClip("Kamae", Kamae);

    AnimationClip KamaeEnd;
    KamaeEnd.frames = { 248, 248 };
    KamaeEnd.speed = 0.1f;
    KamaeEnd.loop = false;
    m_anim->AddClip("KamaeEnd", KamaeEnd);

    AnimationClip KamaeAir;
    KamaeAir.frames = { 250, 251, 251, 252, 253 };
    KamaeAir.speed = 0.1f;
    KamaeAir.loop = false;
    m_anim->AddClip("KamaeAir", KamaeAir);

    AnimationClip KamaeAirEnd;
    KamaeAirEnd.frames = { 254 };
    KamaeAirEnd.speed = 0.1f;
    KamaeAirEnd.loop = false;
    m_anim->AddClip("KamaeAirEnd", KamaeAirEnd);

    AnimationClip JutsuKamae;
    JutsuKamae.frames = { 255, 256, 257, 258, 259 };
    JutsuKamae.speed = 0.1f;
    JutsuKamae.loop = false;
    m_anim->AddClip("JutsuKamae", JutsuKamae);

    AnimationClip JutsuKamaeEnd;
    JutsuKamaeEnd.frames = { 255, 255, 258, 258 };
    JutsuKamaeEnd.speed = 0.1f;
    JutsuKamaeEnd.loop = false;
    m_anim->AddClip("JutsuKamaeEnd", JutsuKamaeEnd);

    AnimationClip executionStart;
    executionStart.frames = { 260, 261, 261, 262, 262, 263, 263, 264, 264, 264, 265, 266, 266 };
    executionStart.speed = 0.1f;
    executionStart.loop = false;
    m_anim->AddClip("executionStart", executionStart);

    AnimationClip executionHorizon;
    executionHorizon.frames = { 267, 268, 268 };
    executionHorizon.speed = 0.1f;
    executionHorizon.loop = false;
    m_anim->AddClip("executionHorizon", executionHorizon);

    AnimationClip executionVertical;
    executionVertical.frames = { 269, 270, 270 };
    executionVertical.speed = 0.1f;
    executionVertical.loop = false;
    m_anim->AddClip("executionVertical", executionVertical);

    AnimationClip executionEnd;
    executionEnd.frames = { 271, 271, 272, 272, 273, 273, 274, 274 };
    executionEnd.speed = 0.1f;
    executionEnd.loop = false;
    m_anim->AddClip("executionEnd", executionEnd);

    AnimationClip KaryuStart;
    KaryuStart.frames = { 259 };
    KaryuStart.speed = 0.1f;
    KaryuStart.loop = false;
    m_anim->AddClip("KaryuStart", KaryuStart);

    AnimationClip KaryuMid;
    KaryuMid.frames = { 275 };
    KaryuMid.speed = 0.1f;
    KaryuMid.loop = false;
    m_anim->AddClip("KaryuMid", KaryuMid);

    AnimationClip KaryuEnd;
    KaryuEnd.frames = { 276, 276, 276, 277, 277, 277, 278, 278, 278 };
    KaryuEnd.speed = 0.1f;
    KaryuEnd.loop = false;
    m_anim->AddClip("KaryuEnd", KaryuEnd);

    m_collision->SetRect(85, 152);
    m_sprite->SetDrawOffset(0.0f, -34.0f);

    // HPが0になったときのコールバックを設定
    //if (m_hp) {
    //    m_hp->OnDeath = [this]() {
    //        std::cout << "Player died! Showing Game Over Menu..." << std::endl;
    //        // PlaySceneを取得してゲームオーバーメニューを表示
    //        PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    //        if (playScene) {
    //            playScene->ShowGameOverMenu();
    //        }
    //        };
    //}

    m_anim->Play("idle");
    m_state = PlayerState::IDLE;

    m_kunaiSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/kunai.wav")
    );
    if (m_kunaiSound != nullptr) m_kunaiSound->SetVolume(180);
    
    m_excusionSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/excusion.wav")
    );
    if (m_excusionSound != nullptr) m_excusionSound->SetVolume(180);

    m_kamaeSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/kamae.wav")
    );
    if (m_kamaeSound != nullptr) m_kamaeSound->SetVolume(180);

    m_karyuSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/karyu.wav")
    );
    if (m_karyuSound != nullptr) m_karyuSound->SetVolume(180);

    m_weakAttackSound1 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack1.wav")
    );
    if (m_weakAttackSound1 != nullptr) m_weakAttackSound1->SetVolume(140);

    m_weakAttackSound2 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack2.wav")
    );
    if (m_weakAttackSound2 != nullptr) m_weakAttackSound2->SetVolume(140);

    m_weakAttackSound3 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack3.wav")
    );
    if (m_weakAttackSound3 != nullptr) m_weakAttackSound3->SetVolume(140);

    m_weakAttackSound4 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack4.wav")
    );
    if (m_weakAttackSound4 != nullptr) m_weakAttackSound4->SetVolume(140);

    m_strongAttackSound1 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack4.wav")
    );
    if (m_strongAttackSound1 != nullptr) m_strongAttackSound1->SetVolume(140);

    m_strongAttackSound2 = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack4.wav")
    );
    if (m_strongAttackSound2 != nullptr) m_strongAttackSound2->SetVolume(140);

    m_hayabusaAttackSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/hayabusa.wav")
    );
    if (m_hayabusaAttackSound != nullptr) m_hayabusaAttackSound->SetVolume(120);

    m_karaburiSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/karaburi.wav")
    );
    if (m_hayabusaAttackSound != nullptr) m_karaburiSound->SetVolume(90);

   


    return true;
}

void PlayerEntity::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    UpdateIgnorePlatform();
    UpdateInvincible(deltaTime);

    UpdateJutsuKamae(deltaTime);
    UpdateKaryu(deltaTime);

    UpdateKamae(deltaTime);
    UpdateExecution(deltaTime);

    UpdateSensor();
    UpdateJump(deltaTime);
    UpdateGravity(deltaTime);

    CheckCanStand();

    UpdateAttack(deltaTime);
    UpdateScale();
    UpdateState(deltaTime);

    if (m_anim) m_anim->Update(deltaTime);

    // デバッグ用：4キーを押すとHPが0になる
#ifdef _DEBUG
    const Input& input = m_scene->GetGame()->GetInput();
    if (input.GetKey().IsTrigger(Key::NUM_4)) {
        if (m_hp) {
            m_hp->Damage(m_hp->GetMaxHP());
            std::cout << "[Debug] Player HP set to 0 by pressing 4 key" << std::endl;
        }
    }
#endif

}

void PlayerEntity::UpdateIgnorePlatform()
{
    if (!m_ignorePlatform)
        return;

    if (!m_ignorePlatformBlock)
        return;

    float platformBottom =
        m_ignorePlatformBlock->GetPos().y +
        m_ignorePlatformBlock->GetCollision()->GetHeight() * 0.5f;

    float playerTop =
        m_transform->GetPosition().y -
        m_collision->GetHeight() * 0.5f;

    if (playerTop > platformBottom)
    {
        m_ignorePlatform = false;
        m_ignorePlatformBlock = nullptr;
    }
}

void PlayerEntity::UpdateInvincible(float deltaTime)
{
    if (m_state == PlayerState::DEAD) return;
    if (m_getHit) {
        m_getHitTimer -= deltaTime;
        if (m_getHitTimer <= 0.0f) {
            m_getHit = false;
            m_canMove = true;
        }
    }

    if (m_invincibleTime > 0.0f)
    {
        m_invincibleTime -= deltaTime;
    }
}

void PlayerEntity::UpdateSensor() {
    float dir = m_dir ? 1.0f : -1.0f;

    m_sensor.front = CheckSensor({ dir * 130.0f, -60.0f });
    m_sensor.frontUpper = CheckSensor({ dir * 130.0f, -60.0f });
    m_sensor.frontBottom = CheckSensor({ dir * 130.0f, 40.0f });
    m_sensor.frontGround = CheckSensor({ dir * 130.0f, 80.0f });
    m_sensor.frontNearGround = CheckSensor({ dir * 60.0f, 80.0f });
}


void PlayerEntity::UpdateJutsuKamae(float deltaTime) {
    if (!m_jutsuCharge) return;
    if (m_getHit) return;
    if (m_dashTimer > 0.0f) return;
    if (!m_isGround) return;
    if (m_isKaryu) return;

    const Input& input = m_scene->GetGame()->GetInput();

    if (input.IsDown(Action::G) && input.IsDown(Action::JUTSU_KAMAE))
    {
        if (m_squat) {
            if (!m_canStand) {
                return;
            }
            else {
                ExitSquat();
            }
        }
        m_isJutsuKamae = true;
        m_canMove = false;
    }
    else
    {
        m_isJutsuKamae = false;
        if (!m_isKaryu) m_canMove = true;
    }
}

void PlayerEntity::UpdateKaryu(float deltaTime) {
    if (m_state == PlayerState::DEAD) return;
    const Input& input = m_scene->GetGame()->GetInput();

    if (m_KaryuTimer <= 0.0f) {
        if (m_isJutsuKamae && input.IsTrigger(Action::KUNAI))
        {
            m_isKaryu = true;
            m_KaryuTimer = 5.0f;
            m_invincibleTime = m_KaryuTimer;
            m_isJutsuKamae = false;
            m_jutsuGauge = 0;
            m_jutsuCharge = false;
            KaryuTextUI* UI = new KaryuTextUI(m_scene);
            m_scene->SpawnUIActor(UI);
        }
        else if (m_isKaryu) {
            m_isKaryu = false;
            m_canMove = true;

            auto enemies = CollectEnemiesInScreen();

            for (EnemyEntity* enemy : enemies)
            {
                m_combo += 5;

                float dir = m_transform->GetPosition().x - enemy->GetPos().x > 0 ? 1.0f : -1.0f;

                enemy->TakeDamage(1400, { dir * 300.0f, 200.0f });
            }
        }
    }
    else {
        m_KaryuTimer -= deltaTime;
    }
}

void PlayerEntity::UpdateKamae(float deltaTime)
{
    if (m_getHit) return;
    if (m_dashTimer > 0.0f) return;
    if (m_isJutsuKamae) { 
        m_isKamae = false;
        return; 
    }
    if (m_isKaryu) return;

    const Input& input = m_scene->GetGame()->GetInput();

    if (input.IsDown(Action::KAMAE))
    {
        if (m_squat) {
            if (!m_canStand) {
                return;
            }else{
                ExitSquat();
            }
        }
        m_isKamae = true;
        m_canMove = false;
    }
    else
    {
        m_isKamae = false;
        if (!m_isExecution) m_canMove = true;
    }
}

void PlayerEntity::UpdateExecution(float deltaTime)
{
    if (m_state == PlayerState::DEAD) return;
    if (!m_isExecution) {
        if (!m_isKamae) return;
    }

    const Input& input = m_scene->GetGame()->GetInput();

    if (input.IsTrigger(Action::DASH))
    {
        if (!m_isExecution) {
        CollectExecutionTargets();
        if (m_executionTargets.empty()) return;
        
            m_isExecution = true;
            m_executionTimer = 0.7f;
            m_invincibleTime = m_executionTimer;
            auto effect = new EffectActor(m_scene, GetPos(), EffectType::ExecutionStart, !m_dir);
            effect->SetFollowTarget(this, { m_dir ? -20.0f : 20.0f, -20.0f });

            SpawnEffect(effect);
        }
    }

    if (m_executionTimer <= 0.0f)
    {
        while (!m_executionTargets.empty())
        {
            EnemyEntity* target = m_executionTargets.back();
            m_executionTargets.pop_back();

            if (target == nullptr)
                continue;

            PlayScene* play = static_cast<PlayScene*>(m_scene);

            if (!play->IsActorAlive(target))
                continue;

            if (target->IsDead())
                continue;

            if (!target->IsMetsu())
                continue;

            Vector2d prevPos = m_transform->GetPosition();
            Vector2d lastPos = target->GetPos();

            m_transform->SetPosition(lastPos);

            target->MetsuAttacked();

            if (m_excusionSound != nullptr) {
                m_excusionSound->Play();
            }

            m_combo++;
            AddJutsuGauge();

            m_executionTimer = m_invincibleTime = 0.2f;

            if (m_executionTargets.empty())
            {
                m_isExecution = false;

                Vector2d dir = lastPos - prevPos;

                if (dir.length() > 0.0f)
                {
                    dir = dir.normalize();
                    dir.y -= 0.3f;

                    m_dir = dir.x > 0.0f ? 1.0f : 0.0f;
                    m_velocity->Set(dir * m_dashSpeed);
                }
            }

            break;
        }

        if (m_executionTargets.empty())
        {
            m_isExecution = false;
        }
    }
    else {
        m_executionTimer -= deltaTime;
    }
}

void PlayerEntity::CollectExecutionTargets() {

    m_executionTargets.clear();
    auto enemies = CollectEnemiesInScreen();

    for (EnemyEntity* enemy : enemies)
    {
        if (enemy->IsMetsu())
            m_executionTargets.push_back(enemy);
    }
}

std::vector<EnemyEntity*> PlayerEntity::CollectEnemiesInScreen()
{
    std::vector<EnemyEntity*> enemies;

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Enemy)
            continue;

        EnemyEntity* enemy = static_cast<EnemyEntity*>(actor);

        Vector2d playerPos = m_transform->GetPosition();
        Vector2d enemyPos = enemy->GetPos();
        if (std::abs(playerPos.x - enemyPos.x) > 700) continue;
        if (std::abs(playerPos.y - enemyPos.y) > 500) continue;
        enemies.push_back(enemy);
    }

    return enemies;
}

void PlayerEntity::UpdateScale() {
    Vector2d scale = m_transform->GetScale();
    scale.x = m_dir ? 1.0f : -1.0f;

    m_transform->SetScale(scale);
}

void PlayerEntity::UpdateJump(float deltaTime) {
    if (!m_canStand) {
        return;
    }
    if (!m_canMove) return;
    if (m_attack) return;

    const Input& input = m_scene->GetGame()->GetInput();
    Vector2d vel = m_velocity->Get();

    if (input.IsTrigger(Action::JUMP)) {
        if (m_squat) {
            ExitSquat();
            m_sprite->SetDrawOffset(0.0f, -34.0f);
            BlockActor* block = CheckSensor({0.0f, m_collision->GetHeight() * 0.5f + 2.0f });
            if (block && block->GetBlockType() == BlockType::Platform)
            {
                SetIgnorePlatform(block);
                return;
            }
        }

        if (m_jumpCount == 0) {
            vel.y = -600.0f;

            m_isGround = false;
            m_jumpCount++;
            m_jumpTime = 0.0f;
            m_collision->SetRect(85, 150);
        }
        else if (m_jumpCount == 1) {
            vel.y = -800.0f;
            m_jumpCount++;
            m_jumpTime = 0.0f;
        }
    }

    if (input.IsDown(Action::JUMP) && m_jumpCount == 1 && m_jumpTime < m_maxJumpTime) {
        vel.y += -1500.0f * deltaTime;
        m_jumpTime += deltaTime;
    }

    if (m_jumpCount >= 2) {
        m_jumpTime += deltaTime;
        if (m_jumpTime > 0.2f) {
            float dir = m_dir ? 1.0f : -1.0f;
            float angle = fabs(m_transform->GetAngle()) + deltaTime * 40;
            if (angle >= 12.6f) angle = 12.6f;

            m_transform->SetAngle(angle * dir);
        }
    }
    m_velocity->Set(vel);
}

void PlayerEntity::UpdateGravity(float deltaTime) {
    if (m_isExecution) {
        m_velocity->SetVelocity({0.0f, 0.0f});
        return;
    }
    MoveAndCollide(deltaTime);
    
    if (m_isGround) {
        m_jumpCount = 0;
        m_transform->SetAngle(0.0f);
        if (!m_squat)
        {
            m_collision->SetRect(85, 152);
        }
        m_HienCount = 0;
    }

    if (m_dashTimer > 0.0f && !m_isGround) {
        Vector2d vel = m_velocity->GetVelocity();
        vel.y = -50.0f;
        m_velocity->SetVelocity(vel);
    }
}

void PlayerEntity::CheckCanStand()
{
    m_canStand = true;

    if (!m_squat)
        return;

    Vector2d standPos = m_transform->GetPosition();

    float squatHeight = m_collision->GetHeight();
    float standHeight = 152.0f;

    standPos.y -= (standHeight - squatHeight) * 0.5f;

    float halfW = 85.0f * 0.5f;
    float halfH = standHeight * 0.5f;

    for (auto actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
            continue;

        auto block = static_cast<BlockActor*>(actor);

        CollisionComponent* blockCol = block->GetCollision();

        Vector2d blockPos = block->GetPos();

        float blockHalfW = blockCol->GetWidth() * 0.5f;
        float blockHalfH = blockCol->GetHeight() * 0.5f;

        float diffX = standPos.x - blockPos.x;
        float diffY = standPos.y - blockPos.y;

        float overlapX = (halfW + blockHalfW) - std::abs(diffX);
        float overlapY = (halfH + blockHalfH) - std::abs(diffY);

        if (overlapX > 0 && overlapY > 0)
        {
            m_canStand = false;
            return;
        }
    }
}

BlockActor* PlayerEntity::CheckSensor(const Vector2d& offset, const Vector2d& size)
{
    Vector2d sensorPos = m_transform->GetPosition() + offset;

    float sensorHalfW = size.x * 0.5f;
    float sensorHalfH = size.y * 0.5f;

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
            continue;

        auto block = static_cast<BlockActor*>(actor);

        Vector2d blockPos = block->GetPos();
        CollisionComponent* col = block->GetCollision();

        if (std::abs(blockPos.x - sensorPos.x) > 500)
            continue;

        if (std::abs(blockPos.y - sensorPos.y) > 300)
            continue;

        float blockHalfW = col->GetWidth() * 0.5f;
        float blockHalfH = col->GetHeight() * 0.5f;

        float diffX = std::abs(sensorPos.x - blockPos.x);
        float diffY = std::abs(sensorPos.y - blockPos.y);

        if (diffX <= sensorHalfW + blockHalfW &&
            diffY <= sensorHalfH + blockHalfH)
        {
            return block;
        }
    }

    return nullptr;
}

void PlayerEntity::EnterSquat()
{
    if (!m_canMove) return;
    float oldHeight = m_collision->GetHeight();
    float newHeight = 95.0f;

    Vector2d pos = m_transform->GetPosition();

    pos.y += (oldHeight - newHeight) * 0.5f;


    m_transform->SetPosition(pos);
    m_collision->SetRect(90, 95);
    m_sprite->SetDrawOffset(0.0f, -62.0f);   // 上に48px
    m_squat = true;
}

void PlayerEntity::ExitSquat()
{
    float oldHeight = m_collision->GetHeight();
    float newHeight = 152.0f;

    Vector2d pos = m_transform->GetPosition();

    pos.y -= (newHeight - oldHeight) * 0.5f;

    m_transform->SetPosition(pos);
    m_collision->SetRect(85, 152);
    m_sprite->SetDrawOffset(0.0f, -34.0f);
    m_squat = false;
}

void PlayerEntity::SpawnKunai()
{
    Vector2d pos = GetPos();

    pos.x += m_dir ? 40.0f : -40.0f;
    pos.y -= 10.0f;

    auto kunai = new KunaiActor(m_scene, pos, m_dir);

    SpawnEffect(kunai);
}

void PlayerEntity::UpdateAttack(float deltaTime) {
    if (m_state == PlayerState::DEAD) return;
    if (m_isKamae) return;
    if (m_isExecution) return;
    if (m_isJutsuKamae) return;
    if (m_isKaryu) return;

    m_hit = false;

    const Input& input = m_scene->GetGame()->GetInput();
    Vector2d pPos = m_transform->GetPosition();
    if (m_canAttack) {
        m_attackType = AttackType::NONE;
        if (input.IsTrigger(Action::KUNAI)) {
            if (m_kunai > 0) {
                m_attack = true;
                m_canAttack = false;
                m_canMove = false;
                if (m_squat) {
                    m_attackType = AttackType::SQUAT_KUNAI;
                    m_attackTimer = 0.5f;
                    m_attackLockTimer = 0.3f;
                    m_kunai--;
                    m_kunaiSpawnTimer = 0.05f; // 約3f
                    m_kunaiPending = true;
                    m_anim->Play("KunaiSquat", true);
                }
                else if (!m_isGround) {
                    m_attackType = AttackType::AIR_KUNAI;
                    m_attackTimer = 0.2f;
                    m_attackLockTimer = 0.1f;
                    m_kunai--;
                    m_kunaiSpawnTimer = 0.05f; // 約3f
                    m_kunaiPending = true;
                    m_anim->Play("KunaiAir", true);
                }
                else {
                    m_attackType = AttackType::KUNAI;
                    m_attackTimer = 0.3f;
                    m_attackLockTimer = 0.3f;
                    m_kunai--;
                    m_kunaiSpawnTimer = 0.05f; // 約3f
                    m_kunaiPending = true;
                    m_anim->Play("Kunai", true);
                }
            }
        }

        if (input.IsTrigger(Action::WEAK_ATTACK) && m_attackType == AttackType::NONE) {
            m_attack = true;
            m_canAttack = false;
            m_canMove = false;
            if (m_squat) {
                m_attackType = AttackType::SQUAT_ATTACK;
                m_attackTimer = 0.4f;
                m_attackLockTimer = 0.2f;
                CheckAttackHit(squatAttack);
                pPos.x += (m_dir ? 90.0f : -90.0f);
                pPos.y -= 50.0f;
                SpawnEffect(new EffectActor(m_scene, pPos, EffectType::SquatAttack, !m_dir));
                m_anim->Play("squatAttack", true);
            }
            else if (!m_isGround) {
                m_attackType = AttackType::AIR_ATTACK;
                m_transform->SetAngle(0.0f);
                switch (m_airAttackIdx) {
                case 0: {
                    m_attackTimer = 0.2f;
                    m_attackLockTimer = 0.2f;
                    CheckAttackHit(airWeak1);
                    if (m_hit) m_airAttackIdx++;
                    auto effect = new EffectActor(m_scene, GetPos(), EffectType::WeakAirAttack1, !m_dir);
                    effect->SetFollowTarget(this, { m_dir ? 70.0f : -70.0f, -30.0f });

                    SpawnEffect(effect);
                    m_anim->Play("weakAirAttack1", true);
                    break;
                }
                case 1: {
                    m_attackTimer = 0.3f;
                    m_attackLockTimer = 0.2f;
                    CheckAttackHit(airWeak2);
                    if (m_hit) m_airAttackIdx++;
                    auto effect = new EffectActor(m_scene, GetPos(), EffectType::WeakAirAttack2, !m_dir);
                    effect->SetFollowTarget(this, { m_dir ? 70.0f : -70.0f, 0.0f });

                    SpawnEffect(effect);
                    m_anim->Play("weakAirAttack2", true);
                    break;
                }
                case 2: {
                    m_attackTimer = 1.3f;
                    m_attackLockTimer = 1.3f;
                    CheckAttackHit(airWeak3);
                    if (m_hit) m_airAttackIdx++;
                    auto effect = new EffectActor(m_scene, GetPos(), EffectType::WeakAirAttack3, !m_dir);
                    effect->SetFollowTarget(this, { m_dir ? 70.0f : -70.0f, -30.0f });

                    SpawnEffect(effect);
                    m_anim->Play("weakAirAttack3", true);
                    break;
                }
                }
            }
            else {
                m_attackType = AttackType::WEAK_ATTACK;
                switch (m_weakAttackIdx) {
                case 0:
                    m_attackTimer = 0.5f;
                    m_attackLockTimer = 0.13f;
                    CheckAttackHit(weak1);
                    pPos.x += (m_dir ? 100.0f : -100.0f);
                    pPos.y -= 50.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::WeakAttack1, !m_dir));
                    m_anim->Play("weakAttack1", true);
                    break;
                case 1:
                    m_attackTimer = 0.4f;
                    m_attackLockTimer = 0.2f;
                    CheckAttackHit(weak2);
                    pPos.x += (m_dir ? 30.0f : -30.0f);
                    pPos.y -= 50.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::WeakAttack2, !m_dir));
                    m_anim->Play("weakAttack2", true);
                    break;
                case 2:
                    m_attackTimer = 0.5f;
                    m_attackLockTimer = 0.25f;
                    CheckAttackHit(weak3);
                    pPos.y -= 50.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::WeakAttack3, !m_dir));
                    m_anim->Play("weakAttack3", true);
                    break;
                case 3:
                    m_attackTimer = 1.0f;
                    m_attackLockTimer = 1.3f;
                    CheckAttackHit(weak4);
                    pPos.x += (m_dir ? 100.0f : -100.0f);
                    pPos.y -= 50.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::WeakAttack4, !m_dir));
                    m_anim->Play("weakAttack4", true);
                    break;
                }
                m_weakAttackIdx++;

                Vector2d vel = m_velocity->Get();
                if (m_sensor.frontNearGround != nullptr) {
                    vel.x = m_dir ? m_moveSpeed : -m_moveSpeed;
                }
                m_velocity->Set(vel);
            }
        }


        if (input.IsTrigger(Action::STRONG_ATTACK) && m_attackType == AttackType::NONE) {
            if (m_squat) {
                if (!m_canStand) {
                    return;
                }
                else {
                    ExitSquat();
                }
            }

            m_attack = true;
            m_canAttack = false;
            m_canMove = false;
            if (!m_isGround) {
                m_transform->SetAngle(0.0f);
                m_HayabusaHit = false;
                m_attackType = AttackType::HAYABUSA;
                m_attackTimer = 0.9f;
                m_attackLockTimer = 0.9f;
                auto effect = new EffectActor(m_scene, GetPos(), EffectType::Hayabusa, !m_dir);
                effect->SetFollowTarget(this, { m_dir ? 70.0f : -70.0f, 100.0f });

                SpawnEffect(effect);
            }
            else {
                m_attackType = AttackType::STRONG_ATTACK;

                switch (m_strongAttackIdx) {
                case 0:
                    m_attackTimer = 0.9f;
                    m_attackLockTimer = 0.4f;
                    CheckAttackHit(strong1);
                    pPos.x += (m_dir ? 70.0f : -70.0f);
                    pPos.y -= 50.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::StrongAttack1, !m_dir));
                    m_anim->Play("strongAttack1", true);
                    break;
                case 1:
                    m_attackTimer = 1.0f;
                    m_attackLockTimer = 1.0f;
                    CheckAttackHit(strong2);
                    pPos.x += (m_dir ? 70.0f : -70.0f);
                    pPos.y -= 60.0f;
                    SpawnEffect(new EffectActor(m_scene, pPos, EffectType::StrongAttack2, !m_dir));
                    m_anim->Play("strongAttack2", true);
                    break;
                }

                m_strongAttackIdx++;

                Vector2d vel = m_velocity->Get();
                if (m_sensor.frontNearGround != nullptr) {
                    vel.x = m_dir ? m_moveSpeed : -m_moveSpeed;
                }
                m_velocity->Set(vel);
            }
        }
    }

    if (!m_canAttack)
    {
        m_attackLockTimer -= deltaTime;

        if (m_attackLockTimer <= 0)
        {
            m_canAttack = true;
        }
    }

    if (m_attack) {
        m_attackTimer -= deltaTime;

        if (m_kunaiPending)
        {
            m_kunaiSpawnTimer -= deltaTime;
            if (m_kunaiSpawnTimer <= 0.0f)
            {
                SpawnKunai();
                m_kunaiPending = false;
            }
        }

        if (m_attackType == AttackType::HAYABUSA) {
            Vector2d vel = m_velocity->Get();
            if (!m_HayabusaHit) {
                CheckAttackHit(Hayabusa);
                if (m_hit) {
                    m_HayabusaHit = true;
                    vel.x = m_dir ? -600.0f : 600.0f;
                    vel.y = -600.0f;
                }
                else {
                    vel.x = m_dir ? m_moveSpeed : -m_moveSpeed;
                }
            }
            m_velocity->Set(vel);
        }

        if (m_attackTimer <= 0) {
            m_attack = false;
            m_canMove = true;
            m_HayabusaHit = false;
            m_weakAttackIdx = 0;
            m_strongAttackIdx = 0;
            m_airAttackIdx = 0;
        }
    }
}

void PlayerEntity::CheckAttackHit(const AttackHitbox& hitbox)
{
    Vector2d attackPos = m_transform->GetPosition();

    if (m_dir)
    {
        attackPos += hitbox.offset;
    }
    else
    {
        attackPos += Vector2d(-hitbox.offset.x, hitbox.offset.y);
    }

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Enemy && actor->GetType() != ActorType::TreasureBox)
        {
            continue;
        }

        EnemyEntity* enemy = static_cast<EnemyEntity*>(actor);

        CollisionComponent* col = enemy->GetCollision();

        if (!col)
        {
            continue;
        }

        float diffX = attackPos.x - enemy->GetPos().x;
        float diffY = attackPos.y - enemy->GetPos().y;

        float overlapX = (hitbox.width * 0.5f + col->GetWidth() * 0.5f) - std::abs(diffX);
        float overlapY = (hitbox.height * 0.5f + col->GetHeight() * 0.5f) - std::abs(diffY);

        if (overlapX > 0 && overlapY > 0)
        {
            if (actor->GetType() == ActorType::TreasureBox) {
                TreasureBoxEntity* enemy = static_cast<TreasureBoxEntity*>(actor);
                enemy->Open();
                continue;
            }
            enemy->TakeDamage( hitbox.damage, { m_dir ? 150.0f : -150.0f, -300.0f });
            enemy->TakeMetsu(hitbox.metsu);
            m_hit = true;
            m_combo++;
            AddJutsuGauge();

            /*SpawnEffect(
                new EffectActor( m_scene, enemy->GetPos(), EffectType::WeakAttack1, !m_dir )
            );*/
        }
    }
}

void PlayerEntity::UpdateState(float deltaTime) {
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (m_hp->GetHP() <= 0) {
        if (m_state == PlayerState::DEAD) {
            if (m_anim->IsFinished()) {
                SetState(Actor::State::Paused);
                if (playScene) {
                    playScene->ShowGameOverMenu();
                }
                return;
            }
        }
        ChangeState(PlayerState::DEAD);
        m_canMove = false;
        return;
    }

    UpdateMove();

    if (m_state == PlayerState::HIT_TRAP) {
        
        playScene->RespawnPlayer();
    }

    if (m_getHit) {
        if (m_isGround) {
            if (!(m_state == PlayerState::HIT_AIR)) {
                ChangeState(PlayerState::HIT_GROUND);
                m_canMove = false;
                printf("ground\n");
                return;
            }
            return;
        }
        else {
            ChangeState(PlayerState::HIT_AIR);
            m_canMove = false;
            printf("air\n");
            return;
        }
    }

    if (m_state == PlayerState::HIT_AIR) {
        if (!m_isGround) {
            return;
        }

        ChangeState(PlayerState::HIT_AIR_LANDING);
        return;
    }

    if (m_state == PlayerState::HIT_AIR_LANDING) {
        if (!(m_anim->IsFinished())) {
            return;
        }
        m_canMove = true;
        m_getHit = false;
    }

    if (m_isKaryu) {
        if (m_state == PlayerState::KARYU_END) {
            if (!m_anim->IsFinished()) return;
            else { m_KaryuTimer = 0.0f; }
        }
        if (m_state == PlayerState::KARYU_MID) {
            if (m_KaryuTimer < 0.5f) {
                ChangeState(PlayerState::KARYU_END);
            }
            return;
        }
        if (m_state == PlayerState::KARYU_START) {
            if (m_KaryuTimer < 3.0f) {
                ChangeState(PlayerState::KARYU_MID);
            }
            return;
        }
        if (m_KaryuTimer > 0.0f) {
            ChangeState(PlayerState::KARYU_START);
            return;
        }
    }

    if (m_isJutsuKamae) {
        ChangeState(PlayerState::JUTSU_KAMAE);
        return;
    }

    if (m_isExecution) {
        if (m_state == PlayerState::EXECUTION_START) {
            if (m_anim->IsFinished()) {
                ChangeState(PlayerState::EXECUTION_HORIZON);
            }
            return;
        }

        if (m_state != PlayerState::EXECUTION_HORIZON && m_state != PlayerState::EXECUTION_VERTICAL) {
            ChangeState(PlayerState::EXECUTION_START);
            return;
        }
        return;
    }

    if (m_state == PlayerState::EXECUTION_HORIZON || m_state == PlayerState::EXECUTION_VERTICAL) {
        ChangeState(PlayerState::EXECUTION_END);
        return;
    }

    if (m_state == PlayerState::EXECUTION_END) {
        if (!m_anim->IsFinished()) {
            m_canMove = true;
            return;
        }
    }

    const Input& input = m_scene->GetGame()->GetInput();

    if (m_isKamae) {
        if (m_isGround) {
            ChangeState(PlayerState::KAMAE);
        }
        else {
            ChangeState(PlayerState::AIR_KAMAE);
        }
        return;
    }

    if (m_attack) {
        if (input.IsTrigger(Action::KUNAI)) {
            switch (m_attackType) {
            case AttackType::SQUAT_KUNAI:
                ChangeState(PlayerState::KUNAI_SQUAT);
                break;
            case AttackType::AIR_KUNAI:
                ChangeState(PlayerState::KUNAI_AIR);
                break;
            case AttackType::KUNAI:
                ChangeState(PlayerState::KUNAI);
                break;
            }
            return;
        }

        if (m_state == PlayerState::SQUAT_ATTACK) {
            if (m_anim->IsFinished())
            {
                ChangeState(PlayerState::SQUAT);
            }
        }

        if (m_squat && !(m_attackType == AttackType::SQUAT_KUNAI)) {
            ChangeState(PlayerState::SQUAT_ATTACK);
            return;
        }

        

        if (input.IsTrigger(Action::WEAK_ATTACK)) {
            if (!m_isGround) {
                switch (m_airAttackIdx) {
                case 0:
                    ChangeState(PlayerState::WEAK_AIR_ATTACK1);
                    break;
                case 1:
                    ChangeState(PlayerState::WEAK_AIR_ATTACK1);
                    break;
                case 2:
                    ChangeState(PlayerState::WEAK_AIR_ATTACK2);
                    break;
                case 3:
                    ChangeState(PlayerState::WEAK_AIR_ATTACK3);
                    break;
                }
                return;
            }

            switch (m_weakAttackIdx) {
            case 1:
                ChangeState(PlayerState::WEAK_ATTACK1);
                break;
            case 2:
                ChangeState(PlayerState::WEAK_ATTACK2);
                break;
            case 3:
                ChangeState(PlayerState::WEAK_ATTACK3);
                break;
            case 4:
                ChangeState(PlayerState::WEAK_ATTACK4);
                m_weakAttackIdx++;
                break;
            }
        }

        if ((m_state == PlayerState::WEAK_ATTACK1) || (m_state == PlayerState::WEAK_ATTACK2) || (m_state == PlayerState::WEAK_ATTACK3)) {
            if (m_anim->IsFinished())
            {
                ChangeState(PlayerState::ATTACK_END);
            }
            return;
        }

        if (input.IsTrigger(Action::STRONG_ATTACK)) {
            if (!m_isGround) {
                ChangeState(PlayerState::HAYABUSA);
                return;
            }

            switch (m_strongAttackIdx) {
            case 1:
                ChangeState(PlayerState::STRONG_ATTACK1);
                break;
            case 2:
                ChangeState(PlayerState::STRONG_ATTACK2);
                m_strongAttackIdx++;
                break;
            }
        }

        if (m_state == PlayerState::STRONG_ATTACK1) {
            if (m_anim->IsFinished())
            {
                ChangeState(PlayerState::STRONG_ATTACK_END);
            }
            return;
        }

        if (m_state == PlayerState::HAYABUSA) {
            if (m_HayabusaHit) {
                ChangeState(PlayerState::HAYABUSA_HIT);
                return;
            }
            if (m_isGround) {
                ChangeState(PlayerState::HAYABUSA_GROUND);
                m_attackTimer = 0;
                return;
            }
            
        }

        if (m_state == PlayerState::HAYABUSA_HIT) {
            if (m_anim->IsFinished())
            {
                ChangeState(PlayerState::FALL);
                m_attackTimer = 0;
                m_attackType = AttackType::WEAK_ATTACK;
            }
            return;
        }

        
        return;
    }

    if (m_state == PlayerState::HAYABUSA_GROUND) {
        if (m_anim->IsFinished())
        {
            ChangeState(PlayerState::JUMP_LANDING);
        }
        return;
    }

    UpdateDash(deltaTime);
    Vector2d move(0.0f, 0.0f);
    float dir = m_dir ? 1.0f : -1.0f;

    if (m_state == PlayerState::ROLL) {
        if (!(m_anim->IsFinished())) {
            if (m_sensor.frontGround == nullptr) {
                move.x = 0.0f;
            }
            else {
                move.x = dir * m_dashSpeed;
            }
            m_velocity->SetX(move.x);
            return;
        }
        ChangeState(PlayerState::ROLL_LANDING);
        return;
    }

    if (m_state == PlayerState::HIEN) {
        if (!(m_anim->IsFinished())) {
            move.x = dir * m_dashAirSpeed;
            m_velocity->SetX(move.x);
            return;
        }
    }

    if (m_state == PlayerState::SENTEN) {
        if (!(m_anim->IsFinished())) {
            move.x = dir * m_dashSpeed;
            m_velocity->SetX(move.x);
            return;
        }
        ChangeState(PlayerState::ROLL_LANDING);
        return;
    }

    UpdateDir(input);
    dir = m_dir ? 1.0f : -1.0f;

    if (input.IsTrigger(Action::DASH)) {
        if (m_isGround) {
            if (m_sensor.frontBottom != nullptr && m_sensor.front == nullptr) {
                ChangeState(PlayerState::SENTEN);
                return;
            }
            if (m_sensor.frontGround != nullptr) {
                ChangeState(PlayerState::ROLL);
                return;
            }
        }
        
        if (!m_HienCount) {
            ChangeState(PlayerState::HIEN);
            return;
        }
    }

    Vector2d vel = m_velocity->Get();

    if (!m_isGround) {
        if (input.IsTrigger(Action::JUMP)) {
            if (m_jumpCount == 1) {
                ChangeState(PlayerState::JUMP_START);
                return;
            }
            else if (m_jumpCount == 2) {
                ChangeState(PlayerState::JUMP_SECOND);
                m_jumpCount++;
                return;
            }
        }
        if (m_state == PlayerState::JUMP_START) {
            if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                if (input.IsDown(Action::LEFT)) {
                    m_velocity->SetX(-m_moveSpeed);
                }
                if (input.IsDown(Action::RIGHT)) {
                    m_velocity->SetX(m_moveSpeed);
                }
            }
            if (m_anim->IsFinished()) {
                ChangeState(PlayerState::JUMP);
            }
            return;
        }

        if (m_state == PlayerState::JUMP) {
            if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                if (input.IsDown(Action::LEFT)) {
                    m_velocity->SetX(-m_moveSpeed);
                }
                if (input.IsDown(Action::RIGHT)) {
                    m_velocity->SetX(m_moveSpeed);
                }
            }
            if (m_anim->IsFinished()) {
                ChangeState(PlayerState::FALL);
            }
            return;
        }

        if (m_state == PlayerState::JUMP_SECOND) {
            if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                if (input.IsDown(Action::LEFT)) {
                    m_velocity->SetX(-m_moveSpeed);
                }
                if (input.IsDown(Action::RIGHT)) {
                    m_velocity->SetX(m_moveSpeed);
                }
            }
            if (m_anim->IsFinished()) {
                ChangeState(PlayerState::FALL);
            }
            return;
        }

        if (m_state == PlayerState::FALL) {
            if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                if (input.IsDown(Action::LEFT)) {
                    m_velocity->SetX(-m_moveSpeed);
                }
                if (input.IsDown(Action::RIGHT)) {
                    m_velocity->SetX(m_moveSpeed);
                }
            }
            return;
        }
        ChangeState(PlayerState::FALL);
        
        return;
    }

    if (m_state == PlayerState::JUMP || m_state == PlayerState::FALL) {
        ChangeState(PlayerState::JUMP_LANDING);
        return;
    }

    if (m_state == PlayerState::JUMP_LANDING) {
        if (!(m_anim->IsFinished())) {
            return;
        }
    }

    if (input.IsDown(Action::DOWN)) {

        if (!m_squat)
        {
            EnterSquat();
            ChangeState(PlayerState::SQUAT_START);
            return;
        }

        if (m_state == PlayerState::SQUAT_START) {
            if (m_anim->IsFinished())
            {
                ChangeState(PlayerState::SQUAT);
            }
            return;
        }

        if (m_state == PlayerState::SQUAT) {
            if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                ChangeState(PlayerState::SQUAT_WALK);
                return;
            }
            return;
        }

        // しゃがみながら移動
        if (m_state == PlayerState::SQUAT_WALK) {
            if (!(input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT))) {
                ChangeState(PlayerState::SQUAT);
                return;
            }

            if (input.IsDown(Action::LEFT)) {
                m_velocity->SetX(-m_moveSpeed * 0.5);
            }
            if (input.IsDown(Action::RIGHT)) {
                m_velocity->SetX(m_moveSpeed * 0.5);
            }
            return;
        }

        ChangeState(PlayerState::SQUAT);
        return;
    }
    

    if (m_squat) {
        if (m_canStand)
        {
            ExitSquat();
        }
        else {
            if (m_state == PlayerState::SQUAT) {
                if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)) {
                    ChangeState(PlayerState::SQUAT_WALK);
                    return;
                }
                return;
            }

            // しゃがみながら移動
            if (m_state == PlayerState::SQUAT_WALK) {
                if (!(input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT))) {
                    ChangeState(PlayerState::SQUAT);
                    return;
                }

                if (input.IsDown(Action::LEFT)) {
                    m_velocity->SetX(-m_moveSpeed * 0.5);
                }
                if (input.IsDown(Action::RIGHT)) {
                    m_velocity->SetX(m_moveSpeed * 0.5);
                }
                return;
            }

            ChangeState(PlayerState::SQUAT);
            return;
        }
    }

    if (m_state == PlayerState::CHANGE_DIR_RUN) {
        if (!(m_anim->IsFinished())) {
            m_velocity->Set(Vector2d(0.0f, 0.0f));
            return;
        }
        ChangeState(PlayerState::RUN);
        return;
    }

    if (m_state == PlayerState::RUN)
    {
        if (m_prevDir != m_dir)
        {
            ChangeState(PlayerState::CHANGE_DIR_RUN);
            return;
        }

        // 移動キー離した
        if (!(input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)))
        {
            ChangeState(PlayerState::STOP_LONG);
            return;
        }

        m_velocity->SetX(m_moveSpeed* dir);
        return;
    }

    if (m_state == PlayerState::RUN_START)
    {
        if (!(input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT)))
        {
            ChangeState(PlayerState::STOP_SHORT);
            return;
        }

        m_velocity->SetX(m_moveSpeed * dir);

        if (m_anim->IsFinished())
        {
            ChangeState(PlayerState::RUN);
        }
        return;
    }

    if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT))
    {
        ChangeState(PlayerState::RUN_START);
        return;
    }

    if ((m_state == PlayerState::STOP_SHORT) || (m_state == PlayerState::STOP_LONG))
    {
        if (input.IsDown(Action::LEFT) ^ input.IsDown(Action::RIGHT))
        {
            ChangeState(PlayerState::RUN);
            return;
        }

        if (m_anim->IsFinished())
        {
            vel = { 0.0f, 0.0f };
            m_velocity->Set(vel);
            ChangeState(PlayerState::IDLE);
            return;
        }

        return;
    }

    ChangeState(PlayerState::IDLE);
}

void PlayerEntity::ChangeState(PlayerState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;

    switch (m_state)
    {
    case PlayerState::IDLE:
        m_anim->Play("idle");
        break;

    case PlayerState::RUN:
    {
        Vector2d move(0.0f, 0.0f); 
        float dir = m_dir ? 1.0f : -1.0f;
        move.x = 1.0f * dir;
        move.x *= m_moveSpeed;

        Vector2d vel = m_velocity->Get();
        vel.x = move.x;
        m_velocity->Set(vel);

        m_anim->Play("run");
    }   break;

    case PlayerState::RUN_START:
        m_anim->Play("runStart");
        break;

    case PlayerState::STOP_SHORT:
    {
        Vector2d vel = m_velocity->Get();
        vel.x *= 0.8f;
        m_velocity->Set(vel);

        m_anim->Play("stopShort");
    }   break;

    case PlayerState::STOP_LONG:
    {
        Vector2d vel = m_velocity->Get();
        vel.x *= 0.8f;
        m_velocity->Set(vel);
        m_anim->Play("stopLong");
    }break;

    case PlayerState::CHANGE_DIR:
        m_anim->Play("changeDir");
        break;

    case PlayerState::CHANGE_DIR_RUN:
    {
        Vector2d scale = m_transform->GetScale();
        scale.x = m_dir ? -1.0f : 1.0f;
        m_transform->SetScale(scale);

        m_anim->Play("changeDirRun");
    }break;
    
    case PlayerState::SQUAT_START:
        m_anim->Play("squatStart");
        break;

    case PlayerState::SQUAT:
        m_anim->Play("squat");
        break;

    case PlayerState::SQUAT_IDLE:
        m_anim->Play("squatIdle");
        break;

    case PlayerState::SQUAT_WALK:
    {
        Vector2d move(0.0f, 0.0f);
        float dir = m_dir ? 1.0f : -1.0f;
        move.x = 0.5f * dir;
        move.x *= m_moveSpeed;

        Vector2d vel = m_velocity->Get();
        vel.x = move.x;
        m_velocity->Set(vel);

        m_anim->Play("squatWalk");
    }   break;

    case PlayerState::SQUAT_ATTACK:
        m_anim->Play("squatAttack");
        break;

    case PlayerState::JUMP_START:
        m_anim->Play("jumpStart");
        break;

    case PlayerState::JUMP:
        m_anim->Play("jump");
        break;

    case PlayerState::JUMP_SECOND:
        m_anim->Play("jumpSecond");
        break;

    case PlayerState::FALL:
        m_anim->Play("fall");
        break;

    case PlayerState::JUMP_LANDING:
        m_anim->Play("jumpLanding");
        break;

    case PlayerState::ROLL:
    {
        Vector2d move(0.0f, 0.0f);
        float dir = m_dir ? 1.0f : -1.0f;
        m_canMove = false;
        m_dashTimer = 0.35f;

        if (m_sensor.frontGround == nullptr){
            move.x = 0.0f;
        }
        else {
            move.x = dir * m_dashSpeed;
        }

        m_velocity->SetX(move.x);

        m_anim->Play("roll");
    }   break;

    case PlayerState::HIEN:
    {
        Vector2d move(0.0f, 0.0f);
        float dir = m_dir ? 1.0f : -1.0f;
        m_canMove = false;
        m_dashTimer = 0.35f;

        move.x = dir * m_dashAirSpeed;
        move.y = -300.0f;
        m_HienCount = 1;
        m_transform->SetAngle(0.0f);

        Vector2d vel = m_velocity->Get();
        vel.x = move.x;
        vel.y += move.y;
        m_velocity->Set(vel);

        m_anim->Play("Hien");
    }   break;

    case PlayerState::SENTEN:
    {
        Vector2d move(0.0f, 0.0f);
        float dir = m_dir ? 1.0f : -1.0f;
        m_canMove = false;
        m_dashTimer = 0.35f;

        move.x = dir * m_dashSpeed;
        move.y = -4400.0f;
        m_isSenten = true;

        Vector2d vel = m_velocity->Get();
        vel.x = move.x;
        vel.y += move.y;
        m_velocity->Set(vel);

        m_anim->Play("Senten");
    }   break;

    case PlayerState::ROLL_LANDING:
        m_anim->Play("rollLanding");
        break;

    case PlayerState::SOU_KUNAI_SENTEN:
        m_anim->Play("SouKunaiSenten");
        break;

    case PlayerState::WALL_HOLD:
        m_anim->Play("wallHold");
        break;

    case PlayerState::WALL_JUMP:
        m_anim->Play("wallJump");
        break;

    case PlayerState::WALL_CLIMB:
        m_anim->Play("wallClimb");
        break;

    case PlayerState::WALL_CLIMB_UP:
        m_anim->Play("wallClimbUp");
        break;

    case PlayerState::PLATFORM_CLIMB:
        m_anim->Play("platformClimb");
        break;

    case PlayerState::CLIMB_END:
        m_anim->Play("climbEnd");
        break;

    case PlayerState::WEAK_ATTACK1:
        m_anim->Play("weakAttack1");
        if (m_weakAttackSound1 != nullptr && m_hit) {
            m_weakAttackSound1->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::WEAK_ATTACK2:
        m_anim->Play("weakAttack2");
        if (m_weakAttackSound2 != nullptr && m_hit) {
            m_weakAttackSound2->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::WEAK_ATTACK3:
        m_anim->Play("weakAttack3");
        if (m_weakAttackSound3 != nullptr && m_hit) {
            m_weakAttackSound3->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::ATTACK_END:
        m_anim->Play("attackEnd");
        break;

    case PlayerState::WEAK_ATTACK4:
        m_anim->Play("weakAttack4");
        if (m_weakAttackSound4 != nullptr && m_hit) {
            m_weakAttackSound4->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::WEAK_AIR_ATTACK1:
        m_anim->Play("weakAirAttack1");
        if (m_weakAttackSound1 != nullptr && m_hit) {
            m_weakAttackSound1->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::WEAK_AIR_ATTACK2:
        m_anim->Play("weakAirAttack2");
        if (m_weakAttackSound2 != nullptr && m_hit) {
            m_weakAttackSound2->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::WEAK_AIR_ATTACK3:
        m_anim->Play("weakAirAttack3");
        if (m_weakAttackSound3 != nullptr && m_hit) {
            m_weakAttackSound3->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::STRONG_ATTACK1:
        m_anim->Play("strongAttack1");
        if (m_strongAttackSound1 != nullptr && m_hit) {
            m_strongAttackSound1->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::STRONG_ATTACK_END:
        m_anim->Play("strongAttackEnd");
        break;

    case PlayerState::STRONG_ATTACK2:
        m_anim->Play("strongAttack2");
        if (m_strongAttackSound2 != nullptr && m_hit) {
            m_strongAttackSound2->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::HAYABUSA:
        m_anim->Play("Hayabusa");
        if (m_hayabusaAttackSound != nullptr && m_hit) {
            m_hayabusaAttackSound->Play();
        }
        else {
            m_karaburiSound->Play();
        }
        break;

    case PlayerState::HAYABUSA_HIT:
        m_anim->Play("HayabusaHit");
        break;
        
    case PlayerState::HAYABUSA_GROUND:
        m_anim->Play("HayabusaGround");
        break;

    case PlayerState::KUNAI:
        m_anim->Play("Kunai");
        if (m_kunaiSound != nullptr)
        {
            m_kunaiSound->Play();
        }
        break;

    case PlayerState::KUNAI_AIR:
        m_anim->Play("KunaiAir");
        if (m_kunaiSound != nullptr)
        {
            m_kunaiSound->Play();
        }
        break;

    case PlayerState::KUNAI_WALL:
        m_anim->Play("wallKunai");
        if (m_kunaiSound != nullptr)
        {
            m_kunaiSound->Play();
        }
        break;

    case PlayerState::KUNAI_SQUAT:
        m_anim->Play("KunaiSquat");
        if (m_kunaiSound != nullptr)
        {
            m_kunaiSound->Play();
        }
        break;

    case PlayerState::HIT_TRAP:
        m_anim->Play("trapHit");
        break;

    case PlayerState::HIT_GROUND:
        m_anim->Play("hitGround");
        break;

    case PlayerState::HIT_AIR:
        m_anim->Play("hitAir");
        break;

    case PlayerState::HIT_AIR_LANDING:
        m_anim->Play("hitAirLanding");
        break;

    case PlayerState::KAMAE:
        m_anim->Play("Kamae");
        
        break;

    case PlayerState::KAMAE_END:
        m_anim->Play("KamaeEnd");
        
        break;

    case PlayerState::AIR_KAMAE:
        m_anim->Play("KamaeAir");
        break;

    case PlayerState::AIR_KAMAE_END:
        m_anim->Play("KamaeAirEnd");
        break;

    case PlayerState::JUTSU_KAMAE:
        m_anim->Play("JutsuKamae");
        break;

    case PlayerState::JUTSU_KAMAE_END:
        m_anim->Play("JutsuKamaeEnd");
        break;

    case PlayerState::EXECUTION_START:
        m_anim->Play("executionStart");
        if (m_kamaeSound != nullptr && !m_kamaeSound->IsPlaying())
        {
            m_kamaeSound->Play();
        }
        break;

    case PlayerState::EXECUTION_HORIZON:
        m_anim->Play("executionHorizon");
        break;

    case PlayerState::EXECUTION_VERTICAL:
        m_anim->Play("executionVertical");
        break;

    case PlayerState::EXECUTION_END:
        m_anim->Play("executionEnd");
        
        break;

    case PlayerState::KARYU_START:
        m_anim->Play("KaryuStart");
        if (m_karyuSound != nullptr)
        {
            m_karyuSound->Play();
        }
        break;

    case PlayerState::KARYU_MID:
        m_anim->Play("KaryuMid");
        break;

    case PlayerState::KARYU_END:
        m_anim->Play("KaryuEnd");
        break;

    case PlayerState::DEAD:
        m_anim->Play("dead");
        break;
    }
}

void PlayerEntity::UpdateMove() {
    Vector2d vel = m_velocity->Get();
    if (vel.x != 0.0f) {
        vel.x *= 0.8f;

        if (std::abs(vel.x) < 1.0f) {
            vel.x = 0.0f;
        }
        m_velocity->Set(vel);
    }
}

void PlayerEntity::UpdateDir(const Input& input) {
    if (m_canMove) {
        m_prevDir = m_dir;
        if (input.IsDown(Action::LEFT)) {
            m_dir = false;
        }
        if (input.IsDown(Action::RIGHT)) {
            m_dir = true;
        }
    }
}

void PlayerEntity::UpdateDash(float deltaTime) {
    if (m_dashTimer > 0.0f) {
        Vector2d vel = m_velocity->Get();

        m_dashTimer -= deltaTime;

        if (m_dashTimer <= 0.0f) {
            m_dashTimer = 0.0f;
            vel.x = 0.0f;
            m_canMove = true;
            m_isSenten = false;
        }

        m_velocity->Set(vel);
    }
}

void PlayerEntity::TakeDamage(int damage, const Vector2d& knockback) {
    if (m_invincibleTime > 0) return;
    if (m_dashTimer > 0.0f) return;

    m_hp->Damage(damage);
    
    m_velocity->Set(knockback);
    
    m_getHit = true;
    m_getHitTimer = 0.5f;
    m_invincibleTime = 1.5f;
    m_combo = 0;
}

void PlayerEntity::HitTrap() {
    ChangeState(PlayerState::HIT_TRAP);
    m_canMove = false;
}

void PlayerEntity::SetMoney(int amount)
{
    int old = m_money;
    m_money = (amount >= 0) ? amount : 0;
    if (OnMoneyChanged) OnMoneyChanged(m_money, old);
}

void PlayerEntity::AddMoney(int delta)
{
    if (delta == 0) return;
    int old = m_money;
    m_money += delta;
    if (m_money < 0) m_money = 0;
    if (OnMoneyChanged) OnMoneyChanged(m_money, old);
}

void PlayerEntity::UpdateDead(float deltaTime) {
    // 画面中央の目標座標（ゲームの画面解像度に合わせて数値を調整してください）
    Vector2d centerPos(960.0f, 540.0f);

    if (!m_isDeadTriggered) {
        m_isDeadTriggered = true;
        m_canMove = false;

        m_sprite->LoadTextureDiv("kari/det.png", 6, 1);
        AnimationClip dead;
        dead.frames = { 0, 1, 2, 3, 4, 5 };
        dead.speed = 0.12f;
        dead.loop = false; // 1回再生したら最後のコマで止まる
        m_anim->AddClip("dead", dead);
        m_anim->Play("dead");

        // 死亡した瞬間に一瞬で画面中央へワープさせる
        m_transform->SetPosition(centerPos);
        m_velocity->Set(Vector2d::Zero());
        // 物理移動を完全に停止
        m_velocity->Set(Vector2d::Zero());
        // 毎フレーム画面中央の位置に完全に固定する
        m_transform->SetPosition(centerPos);

    }
}

std::string PlayerEntity::GetTexturePath() const { return ""; }

void PlayerEntity::ResetStageState()
{
    m_velocity->Set({ 0,0 });

    m_jumpCount = 0;
    m_isGround = false;
    m_canMove = true;
    m_canAttack = true;
    m_attack = false;
    m_dashTimer = 0.0f;
    ChangeState(PlayerState::IDLE);
}

//======================================
// ドロップアイテムから呼ばれる関数
//======================================

//追加
void PlayerEntity::AddCoin(int value)
{
    m_coin += value;
    if (m_coin < 0) m_coin = 0;
}

void PlayerEntity::AddKunai(int value)
{
    m_kunai += value;
    if (m_kunai < 0) m_kunai = 0;
}

void PlayerEntity::AddHaku(int value)
{
    m_haku += value;

    if (m_haku < 0)
        m_haku = 0;

    if (m_haku > m_maxHaku)
        m_haku = m_maxHaku;
}

void PlayerEntity::HealHP(int value)
{
    if (!m_hp) return;
    m_hp->Heal(value);
}

