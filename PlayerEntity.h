#pragma once
#include "EntityActor.h"
#include "BlockActor.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include <functional>
#include"SoundComponent.h"

struct AttackHitbox
{
    Vector2d offset;
    float width = 0;
    float height = 0;
    int damage = 0;
    int metsu = 0;
};

struct SensorData
{
    BlockActor* front = nullptr;
    BlockActor* frontUpper = nullptr;
    BlockActor* frontBottom = nullptr;
    BlockActor* frontGround = nullptr;
    BlockActor* frontNearGround = nullptr;
};

class EnemyEntity;
class VelocityComponent;
class GravityComponent;
class CollisionComponent;
class AnimationComponent;
class HPComponent;
class Input;

class PlayerEntity : public EntityActor {
public:
    enum class PlayerState {
        IDLE,

        RUN_START,
        RUN,
        STOP_SHORT,
        STOP_LONG,

        CHANGE_DIR,
        CHANGE_DIR_RUN,

        SQUAT_START,
        SQUAT,
        SQUAT_IDLE,
        SQUAT_WALK,

        JUMP_START,
        JUMP,
        JUMP_SECOND,
        FALL,
        JUMP_LANDING,

        ATTACK,
        WEAK_ATTACK1,
        WEAK_ATTACK2,
        WEAK_ATTACK3,
        ATTACK_END,
        WEAK_ATTACK4,

        WEAK_AIR_ATTACK1,
        WEAK_AIR_ATTACK2,
        WEAK_AIR_ATTACK3,
        HAYABUSA,
        HAYABUSA_HIT,
        HAYABUSA_GROUND,

        STRONG_ATTACK1,
        STRONG_ATTACK_END,
        STRONG_ATTACK2,

        SQUAT_ATTACK,
        WALL_ATTACK,

        KUNAI,
        KUNAI_AIR,
        KUNAI_SQUAT,
        KUNAI_WALL,

        HIT,
        HIT_GROUND,
        HIT_AIR,
        HIT_AIR_LANDING,
        HIT_TRAP,

        ROLL,
        HIEN,
        SENTEN,
        ROLL_LANDING,

        SOU_KUNAI_SENTEN,

        WALL_HOLD,
        WALL_JUMP,
        WALL_CLIMB,
        WALL_CLIMB_UP,
        PLATFORM_CLIMB,
        CLIMB_END,

        KAMAE,
        KAMAE_END,
        AIR_KAMAE,
        AIR_KAMAE_END,

        JUTSU_KAMAE,
        JUTSU_KAMAE_END,

        EXECUTION_START,
        EXECUTION_HORIZON,
        EXECUTION_VERTICAL,
        EXECUTION_END,

        KARYU_START,
        KARYU_MID,
        KARYU_END,

        DEAD,
    };

    enum class AttackType {
        NONE,
        WEAK_ATTACK,
        STRONG_ATTACK,
        KUNAI,
        SQUAT_ATTACK,
        SQUAT_KUNAI,
        AIR_ATTACK,
        AIR_KUNAI,
        WALL_ATTACK,
        WALL_KUNAI,
        HAYABUSA,
    };

    explicit PlayerEntity(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32,32 });
    ~PlayerEntity() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    void UpdateIgnorePlatform();
    void UpdateInvincible(float deltaTime);
    void UpdateSensor();

    void UpdateJutsuKamae(float deltaTime);
    void UpdateKaryu(float deltaTime);
    bool GetIsKaryu() { return m_isKaryu; }
    float GetKaryuTimer() { return m_KaryuTimer; }

    void UpdateKamae(float deltaTime);
    void UpdateExecution(float deltaTime);
    void CollectExecutionTargets();
    std::vector<EnemyEntity*> CollectEnemiesInScreen();

    void UpdateScale();
    void UpdateJump(float deltaTime);
    void UpdateGravity(float deltaTime);
    void UpdateAttack(float deltaTime);
    void UpdateState(float deltaTime);
    void ChangeState(PlayerState newState);
    void UpdateMove();
    void UpdateDir(const Input& input);
    void UpdateDash(float deltaTime);
    void UpdateDead(float deltaTime); // 死亡時の専用アップデート

    ActorType GetType() const override { return ActorType::Player; }
    CollisionComponent* GetCollision() const { return m_collision; }
    CollisionComponent* GetAttackCol() const { return m_attackCol; }

    void AddJutsuGauge() {
        m_jutsuGauge++;
        if (m_jutsuGauge >=25) {
            m_jutsuGauge = 25;
            m_jutsuCharge = true;
        }
        printf("jutsuGauge: %d\n", m_jutsuGauge);
    }
    bool GetJutsuCharge() { return m_jutsuCharge; }
    int GetJutsuGaugeAmount() const { return m_jutsuGauge; }

    bool OnGround() const { return m_isGround; }
    bool GetCanMove() { return m_canMove; }
    void SetCanMove(bool canMove) { m_canMove = canMove; }
    void SetPosition(Vector2d pos) { m_transform->SetPosition(pos); }

    HPComponent* GetHP() const { return m_hp; }
    bool GetDir() const { return m_dir; }
    int GetCombo() const { return m_combo; }

    bool IgnorePlatform() const override { return m_ignorePlatform; }
    void SetIgnorePlatform(BlockActor* block)
    {
        m_ignorePlatform = true;
        m_ignorePlatformBlock = block;
    }

    void CheckAttackHit(const AttackHitbox& hitbox);
    void TakeDamage(int damage, const Vector2d& knockback);
    void HitTrap();

    void CheckCanStand();
    BlockActor* CheckSensor(const Vector2d& offset, const Vector2d& size = {4.0f, 4.0f});

    void EnterSquat();
    void ExitSquat();

    void SpawnKunai();

    

    Vector2d GetDrawOffset() const { return m_sprite->GetDrawOffset(); }
    // ★追加: 手裏剣の数を返す関数
    int GetShurikenCount() const { return m_kunai; }

    std::function<void(int newMoney, int oldMoney)> OnMoneyChanged; // コールバック
    int GetMoney() const { return m_money; }
    void SetMoney(int amount);
    void AddMoney(int delta);

    void ResetStageState();

    // ===== ドロップアイテム用 =====
    void AddCoin(int value);
    void AddKunai(int value);
    void AddHaku(int value);
    void HealHP(int value);

    int GetCoin() const { return m_coin; }
    int GetKunai() const { return m_kunai; }
    int GetHaku() const { return m_haku; }
    int GetMaxHaku() const { return m_maxHaku; }

private:
    HPComponent* m_hp;
    int m_hpMax;
    GravityComponent* m_gravity;
    //SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    int m_combo;
    int m_kunai;

    PlayerState m_state;

    float m_kunaiSpawnTimer;
    bool m_kunaiPending;

    bool m_dir;
    bool m_prevDir;
    float m_jumpSpeed;    // ジャンプ速度
    float m_moveSpeed;    // 移動速度
    float m_dashSpeed;
    float m_dashAirSpeed;
    float m_dashTimer;
    bool m_HienCount;
    bool m_isSenten;

    // EntityActor.bool m_isGround;          // 接地フラグ
    int m_jumpCount;
    float m_jumpTime;
    float m_maxJumpTime;

    bool m_attack;
    bool m_hit;           // 自身の攻撃が当たったかどうか
    bool m_HayabusaHit;
    AttackType m_attackType;
    CollisionComponent* m_attackCol;

    int m_weakAttackIdx;
    int m_strongAttackIdx;
    int m_airAttackIdx;

    float m_attackTimer;

    bool m_canAttack;
    float m_attackLockTimer;

    bool m_getHit;
    float m_getHitTimer;
    float m_invincibleTime;

    bool m_canMove;       // 移動可否
    bool m_squat;         // しゃがみ
    bool m_canStand;      // しゃがみ可否
    bool m_canCharge;

    int m_jutsuGauge;
    bool m_jutsuCharge;

    bool m_isJutsuKamae;
    bool m_isKaryu;
    float m_KaryuTimer;

    bool m_isKamae;
    bool m_isExecution;
    std::vector<EnemyEntity*> m_executionTargets;
    float m_executionTimer;

    bool m_ignorePlatform;
    BlockActor* m_ignorePlatformBlock = nullptr;

    SensorData m_sensor;

    bool m_isDeadTriggered = false; // 死亡時の初回処理用フラグ

    // ===== ドロップアイテム用の所持値 =====
    int m_money = 0;
    int m_coin;
    int m_haku;
    int m_maxHaku;

    std::string GetTexturePath() const override;

    //サウンド関係

    SoundComponent* m_excusionSound;
    SoundComponent* m_kamaeSound;
    SoundComponent* m_karyuSound;
    SoundComponent* m_kunaiSound;
    SoundComponent* m_weakAttackSound1;
    SoundComponent* m_weakAttackSound2;
    SoundComponent* m_weakAttackSound3;
    SoundComponent* m_weakAttackSound4;

    SoundComponent* m_strongAttackSound1;
    SoundComponent* m_strongAttackSound2;


    SoundComponent* m_hayabusaAttackSound;

    SoundComponent* m_karaburiSound;


};



