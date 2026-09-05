#define NOMINMAX
#include "PlayScene.h"
#include "TitleScene.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "SoundComponent.h"
#include "HPComponent.h"
#include "GameOverMenuUI.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "PlayerEntity.h"
#include "ScarecrowEnemyEntity.h"
#include "WhiteEnemyEntity.h"
#include "YellowEnemyEntity.h"
#include "ArrowEnemyEntity.h"
#include "HealerEnemyEntity.h"
#include "ArmorEnemyEntity.h"
#include "GunnerEnemyEntity.h"
#include "YoroiBossEntity.h"
#include "SekienkiBossEntity.h"
#include "EffectActor.h"
#include "StageBackActor.h"
#include "StageExitActor.h"
#include "GroundBlock.h"
#include "ClearBlock.h"
#include "HouseBlock.h"
#include "YaguraSBlock.h"
#include "YaguraMBlock.h"
#include "YaguraLBlock.h"
#include "YaguraLLBlock.h"
#include "PlatformBlock.h"
#include "StoneSBlock.h"
#include "StoneLBlock.h"
#include "PlatformOboroSBlock.h"
#include "PlatformOboroMBlock.h"
#include "PlatformOboroLBlock.h"
#include "WoodSBlock.h"
#include "WoodLBlock.h"
#include "ClearPlatformBlock.h"
#include "StructureABlock.h"
#include "StructureBBlock.h"
#include "Trap.h"
#include "HPBarUI.h"
#include "BackGroundUI.h"
#include "TreasureBox.h"
#include "JutsuChargeUI.h"
#include "EnemyHPBar.h"
#include "EnemySpawner.h"

//イベントのため変更
#include "EventManager.h"
#include "EventTexture.h"

#include <DxLib.h>
#include <algorithm>

PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_menu(this),
	m_player(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageBgm(nullptr),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1),
	m_bgHandle(0),
	m_fgHandle(0),
	m_eventTexture(std::make_unique<EventTexture>()),
	m_eventManager(std::make_unique<EventManager>(this, m_eventTexture.get())), //イベントのため変更
	m_respawnPos(200, 800),
	m_gameOverMenu(nullptr),
	m_isGameOver(false),
	m_isPaused(false)
{

}

bool PlayScene::Init() {
	m_isRunning = true;
	m_type = Type::Play;
	m_stageIndex = 0;
	//m_lockedSkillIcon = LoadGraph("assets/images/skills/locked.png");
	m_menu.Initialize();
	
	// CSV からマップ読み込み
	if (!m_mapData.LoadStage("assets/maps/stage1")) {
		std::cerr << "Load error map1" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage2")) {
		std::cerr << "Load error map2" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage3")) {
		std::cerr << "Load error map3" << std::endl;
		return false;
	}

	m_mapData.tileSize = 104;

	StageInit(m_stageIndex);

	m_player = new PlayerEntity(this, m_playerSpawnPoints[0], Vector2d({ 152, 64 }));
	AddActor(m_player);

	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);
	m_hpBarUI = hpBar;
	
	ShurikenUI* shuriken = new ShurikenUI(this, 18, 60);
	AddUIActor(shuriken);
	m_shurikenUI = shuriken;

	// プレイヤー所持金 UI（左上に表示）
	m_moneyUI = new MoneyUI(this, m_player, "assets/images/uies/money.png");
	m_moneyUI->SetPosition(20.0f, 110.0f);   // テキスト左上基準（スクリーン座標）
	m_moneyUI->SetImageSize(40.0f, 40.0f);   // 画像を 40x40 px に
	m_moneyUI->SetImageOffset(0.0f, 0.0f);   // 画像の相対オフセット（必要なら微調整）
	m_moneyUI->SetTextOffset(46.0f);         // 画像右側に数字を表示する距離
	m_moneyUI->SetAnchorTopRight(220.0f, 50.0f, -80.0f);// 右上に固定：右端から220px, 上から50px, 画像と数字の間隔を-60px にする
	AddUIActor(m_moneyUI);

	// 忍術チャージUI
	m_jutsuChargeUI = new JutsuChargeUI(this);
	m_jutsuChargeUI->SetNinImagePosition(52.0f, 0.0f);
	m_jutsuChargeUI->SetNinImageScale(0.46f);  // 50%のサイズ（半分）
	AddUIActor(m_jutsuChargeUI);

	// プレイヤー金額変更時に MoneyUI を 3 秒表示（増えたときのみ）
	if (m_player) {
		m_player->OnMoneyChanged = [this](int newMoney, int oldMoney) {
			if (newMoney > oldMoney && m_moneyUI) {
				m_moneyUI->ShowFor(3.0f); // 3秒表示
			}
			};
	}

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg1.png");
	AddBackActor(back);

	EffectActor::LoadEffects();

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	m_stageBgm = m_player->AddComponent<SoundComponent>(
		_T("assets/sounds/bgm.wav")
	);

	if (m_stageBgm != nullptr)
	{
		m_stageBgm->SetVolume(90);
		m_stageBgm->Play(DX_PLAYTYPE_LOOP, true);
	}
	return true;
}

bool PlayScene::StageInit(int stageNo) {
	StageData& stage = m_mapData.stages[stageNo];
	if (m_player != nullptr) m_player->ResetStageState();

	// マップのタイルを配置する
	const float tileSize = static_cast<float>(m_mapData.tileSize);
	const Layer& mapLayer = stage.layers[0];

	for (int y = 0; y < stage.height; ++y) {
		for (int x = 0; x < stage.width; ++x) {
			int tileID = mapLayer.tiles[static_cast<std::vector<int, std::allocator<int>>::size_type>(y) * stage.width + x];
			if (tileID == 0) continue; // 空タイル

			Vector2d pos(x * tileSize, y * tileSize);

			// ここでタイルIDに応じてブロック生成
			// 例：GrassBlock と IceBlock を仮に切り替え
			switch (tileID) {
			case 1:
				AddActor(new GroundBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 2:
			 	AddActor(new ClearBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 5:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new HouseBlock(this, pos, Vector2d(tileSize * 4.0f, tileSize)));
				break;
			case 8:
				AddActor(new YaguraSBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 9:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new YaguraMBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 2.0f)));
				break;
			case 10:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 1.0f);
				AddActor(new YaguraLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.0f)));
				break;
			case 11:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 1.2f);
				AddActor(new YaguraLLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.25f)));
				break;
			case 14:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformBlock(this, pos));
				break;
			case 18:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new StoneSBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize)));
				break;
			case 19:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize - tileSize);
				AddActor(new StoneLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.0f)));
				break;
			case 24:
				pos = Vector2d(x * tileSize + tileSize * 3.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroLBlock(this, pos, Vector2d(tileSize * 8.0f, tileSize * 0.20f)));
				break;
			case 27:
				pos = Vector2d(x * tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				break;
			case 28:
				pos = Vector2d(x * tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				break;
			case 29:
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroMBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.20f)));
				break;
			case 31:
				AddActor(new WoodSBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 32:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize);
				AddActor(new WoodLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize)));
				break;
			case 38:
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 1.5f);
				AddActor(new StructureABlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 4.0f)));
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 4.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 2.0f)));
				break;
			case 39:
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 1.5f);
				AddActor(new StructureBBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 4.0f)));
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 4.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 2.0f)));
				break;
			case 40:
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroMBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.20f)));
				break;
			case 41:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new ClearPlatformBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 0.25f)));
				break;
			case 42:
				pos = Vector2d(x * tileSize - tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize, tileSize * 2.0f)));
				break;
			case 44:
				pos = Vector2d(x * tileSize, y * tileSize + tileSize * 0.625f);
				AddActor(new ClearPlatformBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.25f)));
				break;
			}
		}
	}

	const Layer& objLayer = stage.layers[1];

	m_playerSpawnPoints.clear();

	for (int y = 0; y < stage.height; y++)
	{
		for (int x = 0; x < stage.width; x++)
		{
			int objID =
				objLayer.tiles[static_cast<std::vector<int, std::allocator<int>>::size_type>(y) * stage.width + x];


			Vector2d pos(x * tileSize, y * tileSize);

			switch (objID)
			{
			case 1:
			case 3:
				m_playerSpawnPoints.push_back(pos);
				break;

			case 2:
			{
				pos += Vector2d(0.0f, -tileSize * 2.0f);
				AddActor(new StageExitActor(this, pos, m_stageIndex + 1));
			} break;

			case 4:
			{
				pos += Vector2d(-tileSize * 0.5f, 20.0f);
				AddActor(new TreasureBoxEntity(this, pos));
				break;
			}

			case 9:
				AddActor(new Trap(this, pos, Vector2d(192, 192)));
				break;

			case 14:
			{
				pos += Vector2d(0.0f, -tileSize * 2.0f);
				switch (m_stageIndex) {
				case 1:
					AddActor(new StageBackActor(this, pos, m_stageIndex - 1, 1));
					break;
				case 2:
					AddActor(new StageBackActor(this, pos, m_stageIndex - 1, 4));
					break;
				}
			}
			break;

			case 201:
			{
				AddActor(new ScarecrowEnemyEntity(this, pos));
			}
			break;

			case 202:
			{
				auto* whiteEnemy = EnemySpawner::SpawnEnemy<WhiteEnemyEntity>(this, pos);
			} break;

			case 203:
			{
				auto* yellowEnemy = EnemySpawner::SpawnEnemy<YellowEnemyEntity>(this, pos);
			} break;

			case 204:
			{
				auto* arrowEnemy = EnemySpawner::SpawnEnemy<ArrowEnemyEntity>(this, pos);
			} break;

			case 208:
			{
				auto* healereEnemy = EnemySpawner::SpawnEnemy<HealerEnemyEntity>(this, pos);
			} break;

			case 206:
			{
				auto* armorEnemy = EnemySpawner::SpawnEnemy<ArmorEnemyEntity>(this, pos);
			} break;

			case 207:
			{
				auto* gunnernemy = EnemySpawner::SpawnEnemy<GunnerEnemyEntity>(this, pos);
			} break;

			case 205:
			{
				auto* yoroiBoss = EnemySpawner::SpawnEnemy<YoroiBossEntity>(this, pos, "assets/images/uies/HP_enemy_black.png", Vector2d(192, 192));
			} break;

			case 210:
			{
				auto* sekienkiBoss = EnemySpawner::SpawnEnemy<SekienkiBossEntity>(this, pos, "assets/images/uies/HP_enemy_black.png", Vector2d(192, 192));
			} break;

			default:
				if (objID >= 100)
				{
					AddActor(new EventTrigger(this, pos, Vector2d(tileSize, tileSize), objID, m_eventManager.get()));
				}
				break;
			} // switch
		} // for x
	} // for y

	// ---- ゲームオーバーメニューUI 作成 ----
	m_gameOverMenu = new GameOverMenuUI(this);
	AddUIActor(m_gameOverMenu);

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);


	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera のマップ範囲設定
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	switch (m_stageIndex) {
	case 0:
		m_bgHandle = LoadGraph("assets/images/uies/bg1.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg1.png");
		break;
	case 1:
		m_bgHandle = LoadGraph("assets/images/uies/bg2.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg2.png");
		break;
	case 2:
		m_bgHandle = LoadGraph("assets/images/uies/bg3.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg3.png");
		break;
	}

	return true;
}

void PlayScene::ChangeStage(int index, int spawnIndex) {
	DeleteGraph(m_bgHandle);
	DeleteGraph(m_fgHandle);
	m_stageIndex = index;

	ClearStageActors();
	StageInit(index);

	auto it = std::find(m_actors.begin(), m_actors.end(), m_player);
	if (it != m_actors.end())
	{
		m_actors.erase(it);
		m_actors.push_back(m_player);
	}

	if (spawnIndex >= 0 &&
		spawnIndex < static_cast<int>(m_playerSpawnPoints.size()))
	{
		m_player->SetPosition(m_playerSpawnPoints[spawnIndex]);
		if (m_player)
		{
			Vector2d playerPos = m_player->GetPos();

			Vector2d camPos = playerPos;
			camPos.y -= 150;

			m_camera.SetCenter(camPos);
		}
	}
	if (m_stageBgm != nullptr)
	{
		m_stageBgm->Stop();
		m_stageBgm->Play(DX_PLAYTYPE_LOOP, true);
	}
}

void PlayScene::ClearStageActors()
{
	for (Actor* actor : m_actors)
	{
		switch (actor->GetType())
		{
		case ActorType::Block:
		case ActorType::Enemy:
		case ActorType::Effect:
		case ActorType::StageExit:
		case ActorType::StageBack:
		case ActorType::TreasureBox:
			actor->SetState(Actor::State::Dead);
			break;

		default:
			break;
		}
	}

	// 敵HPバーも消す
	for (auto& pair : m_enemyToHPBarMap)
	{
		if (pair.second)
			pair.second->SetState(Actor::State::Dead);
	}

	m_enemyToHPBarMap.clear();
//#ifdef _DEBUG
//	for (Actor* actor : m_actors)
//	{
//		if (actor->GetType() == ActorType::Block)
//			continue;
//
//		printf("%s\n", typeid(*actor).name());
//	}
//#endif
	m_metsuEnemies.clear();
}

void PlayScene::RequestStageChange(int stage, int spawnIndex)
{
	if (m_requestStageChange || m_fadeState != FadeState::None)
		return;
	m_requestStageChange = true;
	m_nextStage = stage;
	m_nextSpawnIndex = spawnIndex;
	printf("Request Stage %d  fade=%d\n",
		stage,
		(int)m_fadeState);
}

void PlayScene::Update(float deltaTime) {
	// ====== フェード遷移中はゲームロジックを止める ======
	if (m_fadeState != FadeState::None) {
		UpdateFade(deltaTime);
		// フェード中も UI（ゲームオーバーメニュー等）は動かしたいならここで
		updateActors(m_UIactors, deltaTime);
		return; // 通常更新はスキップ
	}

	if (m_requestStageChange)
	{
		m_requestStageChange = false;
		StartFadeToStage(m_nextStage, m_nextSpawnIndex);
	}

	//イベントのため変更
	m_playTimer += deltaTime; //クリアシーンのために追加
	if (m_eventManager->IsRunning())
	{
		m_eventManager->Update(deltaTime);

		if (!m_eventManager->IsBattleEvent())
		{
			return;
		}
	}
	updateActors(m_backactors, deltaTime);


	const Input& input = m_game->GetInput();

	if (input.IsTrigger(Action::MENU))
	{
		if (m_player->GetState() != Actor::State::Paused)
		m_menu.Toggle();

		return;
	}

	if (m_menu.IsOpen())
	{
		m_menu.Update(deltaTime);
		return;
	}

	if (m_shurikenUI && m_player) {
		m_shurikenUI->SetCount(m_player->GetKunai());
	}

	updateActors(m_backactors, deltaTime);
	updateActors(m_actors, deltaTime);
	updateActors(m_UIactors, deltaTime);

	// ゲームオーバーメニューの処理
	if (m_gameOverMenu && m_gameOverMenu->IsActive()) {
		if (m_gameOverMenu->IsDecided()) {
			m_gameOverMenu->ResetDecided();

			switch (m_gameOverMenu->GetSelectedItem()) {
			case GameOverMenuUI::MenuItem::CONTINUE:
				// コンティニュー：リスポーン
				m_isPaused = false;
				m_isGameOver = false;
				m_gameOverMenu->SetActive(false);
				if (m_hpBarUI) {
					m_hpBarUI->SetVisible(true);
				}
				if (m_shurikenUI) {
					m_shurikenUI->SetVisible(true);
				}
				if (m_jutsuChargeUI) {
					m_jutsuChargeUI->SetVisible(true);
				}
				RespawnPlayer();
				break;

			case GameOverMenuUI::MenuItem::WORLD_MAP:
				// ワールドマップ画面へ移動
				// TODO: ワールドマップシーンへの遷移処理を追加
				std::cout << "Transition to World Map (not implemented yet)" << std::endl;
				break;

			case GameOverMenuUI::MenuItem::TITLE:
				// タイトル画面へ移動
			{
				Game* game = m_eventManager->GetGame();
				if (game)
				{
					game->ChangeScene(std::make_unique<TitleScene>(game));
				}
			}	break;

			default:
				break;
			}
		}
	}
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();

		// 中間点をカメラ位置に
		Vector2d camPos = playerPos;
		camPos.y -= 150;
		m_camera.SetCenter(camPos);
		m_camera.SetZoom(1.0f);
		/*
		// 線形補間で徐々にズーム変更
		float currentZoom = m_camera.GetZoom();
		float zoomSpeed = 5.0f;
		float newZoom = currentZoom + (targetZoom - currentZoom) * std::min(zoomSpeed * deltaTime, 1.0f);
		*/

		float fixedCameraY = 400.0f;  // スクロール開始位置の上限

		if (playerPos.y < fixedCameraY) {
			camPos.y = playerPos.y - 200;  // 上に移動したらカメラもスクロール
		}
		else {
			camPos.y = fixedCameraY - 200;  // それ以外はカメラ固定
		}
	}

	// 敵ごとのHPバー追従：World -> Screen using m_camera (PlayScene の m_camera)
	const Vector2d baroffset(-90.0f, -250.0f); // 敵の頭上に表示したければ負の Y オフセット。要調整。
	for (auto it = m_enemyToHPBarMap.begin(); it != m_enemyToHPBarMap.end(); ) {
		EnemyEntity* enemy = it->first;
		EnemyHPBar* hpBar = it->second;

		bool enemyStillPresent = std::find(m_actors.begin(), m_actors.end(), enemy) != m_actors.end();
		bool hpBarStillPresent = std::find(m_UIactors.begin(), m_UIactors.end(), hpBar) != m_UIactors.end();

		if (!enemyStillPresent || !hpBarStillPresent) {
			// どちらか存在しなければマップから除去（まれに両方既に削除済みの場合もある）
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵が存在しない、または死亡していれば UI を消して map から削除
		if (!enemy || enemy->IsDead() || enemy->GetComponent<HPComponent>() == nullptr) {
			if (hpBar) { hpBar->SetState(Actor::State::Dead); }
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵位置取得
		if (hpBar && hpBar->GetState() != Actor::State::Dead) {
			hpBar->SetMetsuValue(enemy->GetMetsuGauge(), enemy->GetMetsuMax());

			auto transform = enemy->GetComponent<TransformComponent>();
			if (transform) {
				Vector2d worldPos = transform->GetPosition() + baroffset;
				// PlayScene のカメラでワールド→スクリーン
				Vector2d screenPos = m_camera.WorldToScreen(worldPos);

				const float barWidth = 63.0f; // SetBarSize と合わせる
				hpBar->SetPosition(screenPos.x - barWidth * 0.5f, screenPos.y);

			}
		}

		++it;
	}
	RemoveDeadActors();

	if (m_shurikenUI && m_player)
	{
		m_shurikenUI->SetCount(m_player->GetShurikenCount());
	}
}
		
void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;
	Vector2d cam = m_camera.GetCenter();

	// 仮背景
	DrawBox(0, 0, 1280, 720, GetColor(200, 200, 200), 1);

	// 背景
	switch (m_stageIndex) {
	case 0:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1130.0f), 1.6f, 1.6f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 1:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1760.0f), 1.45f, 1.45f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 2:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1600.0f), 1.5f, 1.5f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	}
	
	drawActors(m_backactors);
	drawActors(m_actors);

	// 前景
	switch (m_stageIndex) {
	case 0:
		for (int i = 0; i < 19; i++) {
			renderer->DrawSpriteEx(Vector2d(-1290.0f + i * 1600.0f - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 680.0f), 0.8f, 0.8f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		}
		break;
	case 1:
		renderer->DrawSpriteEx(Vector2d(0 - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 4960.0f), 4.3f, 4.3f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 2:
		for (int i = 0; i < 19; i++) {
			renderer->DrawSpriteEx(Vector2d(-1290.0f + i * 1600.0f - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 870.0f), 0.8f, 0.8f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		}
		break;
	}
	
	if (m_player->GetIsKaryu()) {
		float timer = m_player->GetKaryuTimer();
		if (timer > 4.9f) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)((5.0f - timer) / 0.1f * 180)); // 0～255
		}
		else if (timer < 0.5f) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(timer / 0.5f * 180)); // 0～255
		}
		else {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180); // 0～255
		}
		DrawBox(0, 0, 1280, 720, GetColor(150, 20, 20), 1);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// コンボ表示（m_comboCount が 1 以上なら表示）
	if (m_player->GetCombo() > 0) {
		const std::string& debugFont = m_game->GatDebugFont();
		// ここではフォントサイズを大きめ（例 48）で真ん中上に表示
		std::string comboText = std::to_string(m_player->GetCombo()) + " Hits";
		renderer->DrawTextL(Vector2d(20.0f, 120.0f), comboText, Color(0, 0, 0), debugFont, 60, false);
	}

	drawActors(m_UIactors);

	if (m_menu.IsOpen())
	{
		m_menu.Draw();
	}

	if (m_eventManager->IsRunning()) {
		m_eventManager->Draw();
	}


	// 攻撃範囲描画 --------------------------
	/*Vector2d Pos = m_player->GetPos();
	AttackHitbox Weak1{ Vector2d(50,100), 100, 100, 30 };
	if (m_player->GetDir()) {
		Pos.x += Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	} 
	else{
		Pos.x -= Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	}
	renderer->DrawRectCenter(Pos, Weak1.width, Weak1.height, GetColor(0,255,0),false, true);*/
	//------------------------------------------
#ifdef _DEBUG
	// sensor描画
	Vector2d Pos = m_player->GetPos();
	Vector2d offset = { 0.0f, 50.0f };
	if (m_player->GetDir()) {
		Pos.x += offset.x;
		Pos.y += offset.y;
	}
	else {
		Pos.x -= offset.x;
		Pos.y += offset.y;
	}
	renderer->DrawRectCenter(Pos, 4.0f, 4.0f, GetColor(0, 255, 0), false, true);
#endif
	std::vector<NumberInfo> comboInfo = {
		{ (float)m_player->GetCombo(), 0 }
	};


	if (m_resultShown) {
		const std::string& debugFont = m_game->GatDebugFont();

		renderer->DrawNumberFormatW(
			Vector2d(m_game->GetWidth() / 2.4f,
				m_game->GetHeight() / 2.2f),
			Color(0, 0, 0),
			debugFont,
			32,
			"{0} Hits",
			comboInfo,
			false
		);
	}

	// ←ここを追加
	DrawFadeOverlay();

#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(
		Vector2d(m_game->GetWidth() - 150.0f, 0),
		"PlayScene",
		Color(255, 64, 0),
		debugFont,
		24,
		false
	);
#endif
}

bool PlayScene::IsActorAlive(Actor* actor) const
{
	if (actor == nullptr)
		return false;

	return std::find(
		m_actors.begin(),
		m_actors.end(),
		actor
	) != m_actors.end();
}

void PlayScene::AddCombo() {
	m_comboCount++;
	std::cout << "Combo: " << m_comboCount << std::endl;
}

void PlayScene::RespawnPlayer() {
	if (!m_player) return;

	// プレイヤーの位置をリスポーン位置に戻す
	TransformComponent* transform = m_player->GetComponent<TransformComponent>();
	if (transform) {
		transform->SetPosition(m_playerSpawnPoints[0]);
	}

	// プレイヤーの速度をリセット
	VelocityComponent* velocity = m_player->GetComponent<VelocityComponent>();
	if (velocity) {
		velocity->Set(Vector2d::Zero());
	}

	// HPを最大値に回復
	HPComponent* hp = m_player->GetHP();
	if (hp) {
		hp->Heal(hp->GetMaxHP());
	}
	m_player->SetState(Actor::State::Active);
	std::cout << "Player respawned at: " << m_playerSpawnPoints[0].x << ", " << m_playerSpawnPoints[0].y << std::endl;
}

void PlayScene::RegisterEnemyHPBar(EnemyEntity* enemy, EnemyHPBar* hpBar)
{
	if (enemy && hpBar)
	{
		m_enemyToHPBarMap[enemy] = hpBar;
	}
}

void PlayScene::ShowGameOverMenu() {
	m_isGameOver = true;
	m_isPaused = true;

	auto it = std::find(m_UIactors.begin(), m_UIactors.end(), m_gameOverMenu);
	if (it != m_UIactors.end())
	{
		m_UIactors.erase(it);
		m_UIactors.push_back(m_gameOverMenu);
	}

	if (m_hpBarUI) {
		m_hpBarUI->SetVisible(false);
	}

	if (m_shurikenUI) {
		m_shurikenUI->SetVisible(false);
	}

	if (m_jutsuChargeUI) {
		m_jutsuChargeUI->SetVisible(false);
		std::cout << "m_jutsuChargeUI = false;" << std::endl;
	}

	if (m_gameOverMenu) {
		m_gameOverMenu->SetActive(true);
		std::cout << "Game Over Menu displayed" << std::endl;
	}
}

//void PlayScene::SpawnHitEffect(const Vector2d& pos) {
//	m_effect = new HitEffect(this, pos, {32, 32});
//	AddActor(m_effect);
//	std::cout << "Spawned HitEffect at: " << pos.x << ", " << pos.y << std::endl;
//	
//}


// ====================================================
// フェード遷移
// ====================================================

void PlayScene::StartFadeToStage(int idx, int spawnIndex)
{
	// すでに遷移中なら無視
	if (m_fadeState != FadeState::None) return;

	m_pendingStageIndex = idx;
	m_nextSpawnIndex = spawnIndex;
	m_fadeState = FadeState::FadeOut;
	m_fadeTimer = 0.0f;

	// プレイヤーの動きを止める（暗転中は動かないほうが自然）
	if (m_player) {
		if (auto vel = m_player->GetComponent<VelocityComponent>()) {
			Vector2d v = vel->Get();
			v.x = 0.0f;
			vel->Set(v);
		}
	}
}

void PlayScene::UpdateFade(float deltaTime)
{
	m_fadeTimer += deltaTime;

	switch (m_fadeState)
	{
	case FadeState::FadeOut:
		// 徐々に真っ黒に
		if (m_fadeTimer >= FADE_OUT_DURATION) {
			// 真っ黒になった瞬間にステージを構築（見えないので違和感なし）
			m_stageIndex = m_pendingStageIndex;
			ChangeStage(m_nextStage, m_nextSpawnIndex);
			m_pendingStageIndex = -1;
			m_fadeState = FadeState::Hold;
			m_fadeTimer = 0.0f;
		}
		break;

	case FadeState::Hold:
		// 黒画面を一定時間ホールド
		if (m_fadeTimer >= FADE_HOLD_DURATION) {
			m_fadeState = FadeState::FadeIn;
			m_fadeTimer = 0.0f;
		}
		break;

	case FadeState::FadeIn:
		// 徐々に明るく
		if (m_fadeTimer >= FADE_IN_DURATION) {
			m_fadeState = FadeState::None;
			m_fadeTimer = 0.0f;
		}
		break;

	default:
		break;
	}
}

void PlayScene::DrawFadeOverlay()
{
	if (m_fadeState == FadeState::None) return;

	Renderer* renderer = m_game->GetRenderer();

	float t = 0.0f;

	switch (m_fadeState)
	{
	case FadeState::FadeOut:
		// 0 → 1
		t = m_fadeTimer / FADE_OUT_DURATION;
		break;

	case FadeState::Hold:
		// 完全に黒
		t = 1.0f;
		break;

	case FadeState::FadeIn:
		// 1 → 0
		t = 1.0f - (m_fadeTimer / FADE_IN_DURATION);
		break;

	default:
		return;
	}

	// クランプ
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;

	int alpha = static_cast<int>(t * 255.0f);
	renderer->DrawFullScreenFill(Color(0, 0, 0), alpha);
}
