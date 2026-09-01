#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include "Camera.h"
#include "MapData.h"
#include "ShurikenUI.h"
#include "MoneyUI.h"

#include "Menu.h"

#include <memory>
#include <unordered_map>
#include <vector>

class PlayerEntity;
class EnemyEntity;
class HPBarUI;
class EnemyHPBar;
class SoundComponent;
class GameOverMenuUI;
class JutsuChargeUI;

//イベントのため追加
class EventTexture;
class EventManager;

class PlayScene : public Scene
{
public:
	//	コンストラクタ
	PlayScene(class Game* game);
	//	デストラクタ
	~PlayScene() override = default;

	bool Init() override;
	bool StageInit(int stageNo);

	void ChangeStage(int index, int spawnIndex);
	void ClearStageActors();
	void RequestStageChange(int stage, int spawnIndex);

	//	更新
	void Update(float deltaTime) override;
	void Draw() override;

	PlayerEntity* GetPlayer() const { return m_player; }

	// 衝突イベント時に呼ぶ
	//void SpawnHitEffect(const Vector2d& pos);

	bool IsActorAlive(Actor* actor) const;

	void AddCombo();

	int GetCombo() const { return m_comboCount; }

	bool IsResult() const { return m_resultShown; }
	Camera& GetCamera() { return m_camera; }
	const Camera& GetCamera() const { return m_camera; }
	Game* GetGame() const { return m_game; }

	// 最初の地面のY座標を取得
	float GetInitialGroundY() const { return m_initialGroundY; }

	//クリアシーンのため追加　クリアタイムのゲッター関数
	float GetPlayTime() const { return m_playTimer; }

	// プレイヤーをリスポーン位置に戻す
	void RespawnPlayer();

	//敵HPバー処理に使用
	void AddUIActorFromExternal(UIActor* uiActor) { AddUIActor(uiActor); }
	void RegisterEnemyHPBar(EnemyEntity* enemy, EnemyHPBar* hpBar);

	// ゲームオーバー時の処理
	void ShowGameOverMenu();

	int m_bgHandle;
	int m_fgHandle;
private:
	
	PlayerEntity* m_player;
	MapData m_mapData;
	std::vector<Vector2d> m_playerSpawnPoints;
	std::vector<EnemyEntity*> m_metsuEnemies;
	bool m_requestStageChange = false;
	int m_nextStage = 0;
	int m_nextSpawnIndex = 0;

	Menu m_menu;

	Camera m_camera;
	SoundComponent* m_stageBgm;

	int m_stageIndex;
	int m_comboCount = 0;
	float m_comboTimer = 0.0f; // コンボ表示の残り時間（秒）
	const float COMBO_DISPLAY_TIME = 2.0f; // コンボ表示時間（秒）
	int m_currentStage;
	bool m_resultShown = false;

	HPBarUI* m_hpBarUI = nullptr;
	ShurikenUI* m_shurikenUI = nullptr;
	MoneyUI* m_moneyUI = nullptr;

	JutsuChargeUI* m_jutsuChargeUI = nullptr;

	std::unordered_map<EnemyEntity*, EnemyHPBar*> m_enemyToHPBarMap;
	
	//イベントのため変更
	std::unique_ptr<EventTexture> m_eventTexture;
	std::unique_ptr<EventManager> m_eventManager;
	float m_playTimer{ 0.0f }; //クリアタイムのためのカウンタ変数

	// リスポーン位置（初期位置）
	Vector2d m_respawnPos;

	// ゲームオーバーメニュー
	GameOverMenuUI* m_gameOverMenu;
	bool m_isGameOver;
	bool m_isPaused;  // ゲームが一時停止中か

	// 最初に配置された地面のY座標
	float m_initialGroundY = 0.0f;

	// ====== フェード遷移用 ======
	enum class FadeState {
		None,       // 通常プレイ中
		FadeOut,    // 暗転中（次ステージへ）
		Hold,       // 完全な黒画面のホールド
		FadeIn      // 明転中（新ステージ開始）
	};

	FadeState m_fadeState = FadeState::None;
	float     m_fadeTimer = 0.0f;   // 現在のフェード状態の経過時間
	int       m_pendingStageIndex = -1; // フェードアウト後に切り替えるステージ番号

	// 各フェーズの秒数（動画に合わせて調整）
	static constexpr float FADE_OUT_DURATION = 0.35f;
	static constexpr float FADE_HOLD_DURATION = 0.10f;
	static constexpr float FADE_IN_DURATION = 0.35f;

	// フェード遷移を開始する（次ステージへ）
	void StartFadeToStage(int idx, int spawnIndex);
	// フェード状態を更新
	void UpdateFade(float deltaTime);
	// フェードのオーバーレイを描画
	void DrawFadeOverlay();
};