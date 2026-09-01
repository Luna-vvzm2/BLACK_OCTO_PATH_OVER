#include <fstream>
#include <sstream>
#include <algorithm>
#include "EventManager.h"
#include "EventTexture.h"
#include "Scene.h"
#include "Input.h"
#include "PlayScene.h"
#include "ClearScene.h"
#include "PlayerEntity.h"
#include "Game.h"
#include "Actor.h"
#include "WhiteEnemyEntity.h"
#include "YellowEnemyEntity.h"
#include "ArrowEnemyEntity.h"
#include "HealerEnemyEntity.h"
#include "ArmorEnemyEntity.h"
#include "GunnerEnemyEntity.h"
#include "YoroiBossEntity.h"
#include "SekienkiBossEntity.h"
#include "HPComponent.h"
#include "EnemySpawner.h"

inline std::vector<std::string> SplitCSV(const std::string& line)
{
	std::vector<std::string> tokens;
	std::stringstream ss(line);
	std::string token;
	while (std::getline(ss, token, ','))
	{
		tokens.push_back(token);
	}
	return tokens;
}

EventManager::EventManager(Scene* scene, EventTexture* eventTexture)
	: m_scene(scene)
	, m_eventTexture(eventTexture)
{
}

EventManager::~EventManager() = default;

void EventManager::Init(std::unique_ptr<EventBase> event)
{
	
	if (!event) return;
	m_eventQueue.clear();
	m_eventQueue.push_back(std::move(event));
	m_currentEventIndex = 0;

	m_eventQueue[m_currentEventIndex]->Init();
}

void EventManager::Update(float deltaTime)
{
	if (m_currentEventIndex >= m_eventQueue.size()) return;
	m_eventQueue[m_currentEventIndex]->Update(deltaTime);

	if (m_eventQueue[m_currentEventIndex]->IsEnd())
	{
		m_eventQueue[m_currentEventIndex]->End();
		m_currentEventIndex++;

		if(m_currentEventIndex < m_eventQueue.size())
		{
			m_eventQueue[m_currentEventIndex]->Init();
		}
		else
		{
			m_eventQueue.clear();
			m_currentEventIndex = 0;
		}
	}
}

void EventManager::End()
{
	if (m_currentEventIndex < m_eventQueue.size())
	{
		m_eventQueue[m_currentEventIndex]->End();
	}

	m_eventQueue.clear();
	m_currentEventIndex = 0;
}

bool EventManager::IsRunning() const
{
	return m_currentEventIndex < m_eventQueue.size();
}

bool EventManager::IsBattleEvent() const
{
	if (m_currentEventIndex >= m_eventQueue.size()) return false;

	if (m_eventQueue[m_currentEventIndex]->GetType() == EventType::Battle)
	{
		auto battleEvent = static_cast<BattleEvent*>(m_eventQueue[m_currentEventIndex].get());
		return true;
	}

	return false;
}

bool EventManager::IsEventTriggered(int eventId) const
{
	return m_triggeredEvents.find(eventId) != m_triggeredEvents.end();
}

void EventManager::RegisterEvent(int eventId)
{
	m_triggeredEvents.insert(eventId);
}

void EventManager::LoadEventTimeLine(const std::string& filePath, const Vector2d& triggerPos)
{
	m_eventQueue.clear();
	m_currentEventIndex = 0;

	std::ifstream ifs(filePath);
	if (!ifs.is_open()) return;

	std::string text;
	std::string eventType = "TALK"; //デフォルトのイベントタイプ
	std::vector<std::string> eventTexts;

	auto pushEvent = [&]()
		{
			if (eventTexts.empty()) return;

			if (eventType == "TALK")
			{
				m_eventQueue.push_back(std::make_unique<TalkEvent>(m_scene, eventTexts, this));
			}
			else if (eventType == "BATTLE")
			{
				auto battleEvent = std::make_unique<BattleEvent>(m_scene, eventTexts, this);
				battleEvent->SetTrigger(triggerPos);
				m_eventQueue.push_back(std::move(battleEvent));
			}
			else if (eventType == "CUTIN")
			{
				m_eventQueue.push_back(std::make_unique<CutInEvent>(m_scene, eventTexts, this));
			}
			else if (eventType == "CLEAR")
			{
				m_eventQueue.push_back(std::make_unique<ClearEvent>(m_scene, eventTexts, this));
			}
			eventTexts.clear();
		};

	bool isHeaderLine = true; //一行目をスキップするフラグ

	while (std::getline(ifs, text))
	{
		std::istringstream i_stream(text);
		if (text.empty() || text.rfind("//", 0) == 0)continue;

		if (isHeaderLine)
		{
			isHeaderLine = false;
			continue;
		}

		std::stringstream ss(text);
		std::string type;
		std::getline(ss, type, ',');

		if (!type.empty())
		{
			pushEvent();
			eventType = type;
		}
		eventTexts.push_back(text);
	}

	pushEvent();
	ifs.close();

	if(!m_eventQueue.empty())
	{
		m_eventQueue[0]->Init();
	}
}

Game* EventManager::GetGame() const
{
	if (!m_scene) return nullptr;
	return m_scene->GetGame();
}

void EventManager::Draw()
{
	if (m_currentEventIndex < m_eventQueue.size())
	{
		m_eventQueue[m_currentEventIndex]->Draw();
	}
}

EventTexture* EventManager::GetEventTexture() const
{
	return m_eventTexture;
}

EventBase::EventBase() = default;
EventBase::~EventBase() = default;

void EventBase::LoadTexts(const std::string& filePath)
{
	DeleteTexts();

	std::ifstream ifs(filePath);

	if (!ifs.is_open())
	{
		std::cerr << "テキストファイルを読み込めませんでした\n";
		return;
	}

	std::string line;

	while (std::getline(ifs, line))
	{
		if (line.empty() || line.rfind("//", 0) == 0) continue; //空の行とコメント行を無視
		m_texts.push_back(line);
	}
	ifs.close();
}

void EventBase::DeleteTexts()
{
	m_texts.clear();
	m_texts.shrink_to_fit();
}


TalkEvent::TalkEvent(Scene* scene, const std::string& filePath, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	LoadTexts(filePath);
}

TalkEvent::TalkEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	m_texts = texts;
}

TalkEvent::~TalkEvent() = default;

void TalkEvent::Init()
{
	for (auto actor : m_scene->GetActors())
	{
		actor->SetState(Actor::State::Paused);
	}

	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();

		if (texManager)
		{
			for (const std::string& text : m_texts)
			{
				auto tokens = SplitCSV(text);
				if(tokens.size() > 3 && !tokens[3].empty() && tokens[3] != "None")
				{
					texManager->LoadTexture(tokens[3]);
				}
			}
		}

		m_currentLine = 0;
		m_isEnd = false;
		ShowText();
	}
}

void TalkEvent::Update(float deltaTime)
{
	const Input& input = m_scene->GetGame()->GetInput();
	if (input.IsTrigger(Action::ENTER))
	{
		NextText();
	}
	if (m_isTextTimerActive)
	{
		m_textTimer -= deltaTime;
		if (m_textTimer <= 0.0f)
		{
			m_isTextTimerActive = false;
			NextText();
		}
	}

	if (input.IsDown(Action::ESCAPE)) //スキップ処理
	{
		if (input.GetPressFrame(Action::ESCAPE) >= m_skipTimer)
		{
			m_isEnd = true;
		}
	}
}

void TalkEvent::End()
{
	for (auto actor : m_scene->GetActors())
	{
		actor->SetState(Actor::State::Active);
	}
	 
	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();
		if (texManager)
		{
			texManager->Clear();
		}
	}
	DeleteTexts();
}

bool TalkEvent::IsEnd() const
{
	return m_isEnd;
}

void TalkEvent::ShowText()
{
	if (m_currentLine < 0 || m_currentLine >= static_cast<int>(m_texts.size())) return;

	const std::string& text = m_texts[m_currentLine];
	auto tokens = SplitCSV(text);

	if(tokens.size() > 1 && !tokens[1].empty() && tokens[1] != "None")
	{
		m_talkerName = tokens[1];
	}

	if (tokens.size() > 2 && !tokens[2].empty() && tokens[2] != "None")
	{
		m_textTimer = std::stof(tokens[2]);
		m_isTextTimerActive = true;
	}

	if (tokens.size() > 3 && !tokens[3].empty() && tokens[3] != "None")
	{
		if(tokens[3] == "R") m_talkerPosition = ActorPosition::Right;
		else if (tokens[3] == "L") m_talkerPosition = ActorPosition::Left;
	}

	if(tokens.size() > 4 && !tokens[4].empty() && tokens[4] != "None")
	{
		if (m_eventManager && m_eventManager->GetEventTexture())
		{
			m_actorTextureId = m_eventManager->GetEventTexture()->LoadTexture(tokens[4]);
		}

		//画像のサイズを変更できる機能（必要ないかも）
		if (tokens.size() > 6 && !tokens[6].empty() && tokens[6] != "None")
		{
			m_actorW = std::stoi(tokens[6]);
		}
		else
		{
			m_actorW = 480;
		}

		if (tokens.size() > 7 && !tokens[7].empty() && tokens[7] != "None")
		{
			m_actorH = std::stoi(tokens[7]);
		}
		else
		{
			m_actorH = 510;
		}
	}

	if(tokens.size() > 5 && !tokens[5].empty() && tokens[5] != "None")
	{
		m_talkText = tokens[5];
	}


	//改行処理
	std::string target = "<br>";
	size_t pos = m_talkText.find(target);
	while (pos != std::string::npos)
	{
		m_talkText.replace(pos, target.length(), "\n");
		pos = m_talkText.find(target, pos + 1);
	}

}

void TalkEvent::NextText()
{
	m_currentLine++;
	if (m_currentLine >= static_cast<int>(m_texts.size()))
	{
		m_isEnd = true;
	}
	else
	{
		ShowText();
	}
}

void TalkEvent::Draw()
{
	//テキストボックス描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_boxColor.a);
	DrawBox(m_boxX, m_boxY, m_boxW + m_boxX, m_boxH + m_boxY, m_boxColor.ToDxColor(), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//テキストボックスの枠線描画
	DrawBox(m_boxX, m_boxY, m_boxW + m_boxX, m_boxH + m_boxY, m_nameColor.ToDxColor(), FALSE);

	//立ち絵の描画
	if (m_actorTextureId != -1)
	{
		if (m_talkerPosition == ActorPosition::Left)
		{
			int actorX = m_boxX - 220;
			int actorY = (m_boxY + m_boxH) - m_actorH + 50;
			DrawExtendGraph(actorX, actorY, actorX + m_actorW, actorY + m_actorH, m_actorTextureId, TRUE);
		}
		else if (m_talkerPosition == ActorPosition::Right) 
		{
			int actorX = (m_boxX + m_boxW) - m_actorW + 280;
			int actorY = (m_boxY + m_boxH) - m_actorH + 50;
			DrawExtendGraph(actorX, actorY, actorX + m_actorW, actorY + m_actorH, m_actorTextureId, TRUE);
		}

	}

	int textX = m_boxX + 30;
	int textY = m_boxY + 30;

	if (m_actorTextureId > 0 && m_talkerPosition == ActorPosition::Left)
	{
		textX += (m_actorW - 250);
	}

	//ChangeFont("HGP 明朝 E");
	SetFontSize(m_fontSize);
	DrawString(textX, textY, m_talkText.c_str(), m_textColor.ToDxColor());

	if (m_talkerName.empty()) return;
	
	if (m_talkerPosition == ActorPosition::Right)
	{
		m_nameX = m_boxX;
	}
	else
	{
		m_nameX = m_nameX2;
	}

	DrawString(m_nameX, m_nameY, m_talkerName.c_str(), m_nameColor.ToDxColor());
}


BattleEvent::BattleEvent(Scene* scene, const std::string& filePath, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	LoadTexts(filePath);
}

BattleEvent::BattleEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	m_texts = texts;
}

BattleEvent::~BattleEvent() = default;

void BattleEvent::Init()
{
	m_areaXMin = -64.0f;
	m_areaXMax = 1016.0f;

	for (auto actor : m_scene->GetActors())
	{
		if (actor->GetType() == ActorType::Player)
		{
			Vector2d playerPos = actor->GetComponent<TransformComponent>()->GetPosition();
		}
	}
	if (m_eventManager)
	{
		Vector2d triggerPos = m_eventManager->GetTriggerPosition();

		m_areaXMin += triggerPos.x;
		m_areaXMax += m_areaXMin;

		m_isAreaSet = true;
	}
	m_isPlayerInsideArea = false;
	m_isSpawned = false;
	m_isEnd = false;
}

void BattleEvent::Update(float deltaTime)
{
	if (!m_isSpawned)
	{
		for (const std::string& text : m_texts)
		{
			auto tokens = SplitCSV(text);
			if (tokens.size() > 1 && !tokens[1].empty() && tokens[1] != "None")
			{
				Vector2d triggerPos = m_eventManager->GetTriggerPosition();
				m_areaXMin = std::stof(tokens[1]);
				m_areaXMin += triggerPos.x;
			}

			if (tokens.size() > 2 && !tokens[2].empty() && tokens[2] != "None")
			{
				m_areaXMax = std::stof(tokens[2]);
				m_areaXMax += m_areaXMin;
			}

			if (tokens.size() > 3 && !tokens[3].empty() && tokens[3] != "None")
			{
				m_enemyType = std::stoi(tokens[3]);
			}

			if (tokens.size() > 4 && !tokens[4].empty() && tokens[4] != "None")
			{
				m_enemyPos.x = std::stof(tokens[4]);
			}

			if (tokens.size() > 5 && !tokens[5].empty() && tokens[5] != "None")
			{
				m_enemyPos.y = -std::stof(tokens[5]);
			}
			EnemySpawn();
		}
		m_isSpawned = true;
		std::cerr << "生成された敵の数: " << m_actors.size() << "\n";
		return;
	}

	const auto& sceneActors = m_scene->GetActors();

	auto newEnd = std::remove_if(m_actors.begin(), m_actors.end(), [&](Actor* enemy)
		{
			if (enemy == nullptr) return true;
			auto it = std::find(sceneActors.begin(), sceneActors.end(), enemy);
			if (it == sceneActors.end() || enemy->GetState() == Actor::State::Dead)
			{
				return true;
			}
			return false;
		});
	
	m_actors.erase(newEnd, m_actors.end());

	if (m_actors.empty())
	{
		std::cerr << "敵が全滅したためイベントを終了します\n";
		m_isEnd = true; 
		return;
	}

	if (m_isAreaSet)
	{
		//プレイヤーの処理
		for (auto actor : m_scene->GetActors())
		{
			if (actor->GetType() == ActorType::Player)
			{
				if (actor->GetState() == Actor::State::Dead)//まだ不完全 プレイヤーが死んでもDeadになっていない?
				{
					m_isPlayerInsideArea = false;
					continue;
				}

				auto transform = actor->GetComponent<TransformComponent>();
				if (!transform) continue;

				Vector2d pos = transform->GetPosition();

				if (!m_isPlayerInsideArea)
				{
					if (pos.x >= m_areaXMin && pos.x <= m_areaXMax)
					{
						m_isPlayerInsideArea = true; 
					}
				}
				else
				{
					if (pos.x < m_areaXMin) pos.x = m_areaXMin;

					if (pos.x > m_areaXMax) pos.x = m_areaXMax;

					transform->SetPosition(pos);
				}
			}
		}

		//イベント内の敵を閉じ込める処理
		auto clampInside = [this](Actor* actor)
			{
				if (!actor || actor->GetState() == Actor::State::Dead) return;

				auto transform = actor->GetComponent<TransformComponent>();
				if (!transform) return;

				Vector2d pos = transform->GetPosition();

				if (pos.x < m_areaXMin) pos.x = m_areaXMin;
				if (pos.x > m_areaXMax) pos.x = m_areaXMax;

				transform->SetPosition(pos);
			};

		for (auto enemy : m_actors)
		{
			clampInside(enemy);
		}

		//エリア外部の敵を侵入させない処理
		float areaCenter = (m_areaXMin + m_areaXMax) * 0.5f;

		for (auto actor : m_scene->GetActors())
		{
			if (actor->GetType() == ActorType::Enemy && actor->GetState() != Actor::State::Dead)
			{
				if (std::find(m_actors.begin(), m_actors.end(), actor) != m_actors.end())
				{
					continue;
				}

				auto transform = actor->GetComponent<TransformComponent>();
				if (!transform) continue;

				Vector2d pos = transform->GetPosition();

				if (pos.x >= m_areaXMin && pos.x <= m_areaXMax)
				{
					if (pos.x < areaCenter)
					{
						pos.x = m_areaXMin - 1.0f;
					}
					else
					{
						pos.x = m_areaXMax + 1.0f;
					}

					transform->SetPosition(pos);
				}
			}
		}
	}
}


void BattleEvent::End()
{
	DeleteTexts();
}

bool BattleEvent::IsEnd() const
{
	return m_isEnd;
}

void BattleEvent::EnemySpawn()
{
	Vector2d triggerPos{ 0.0f, 0.0f };
	if (m_eventManager)
	{
		triggerPos = m_eventManager->GetTriggerPosition();
	}
	Vector2d spawnPos = m_enemyPos + triggerPos;
	
	auto* playScene = dynamic_cast<PlayScene*>(m_scene);
	if (!playScene)
	{
		std::cerr << "PlayScene 以外のシーンで EnemySpawn が実行されました。\n";
		return;
	}
	Actor* spawnedEnemy = nullptr;

	switch (m_enemyType)
	{
	case 1:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<WhiteEnemyEntity>(playScene, spawnPos);
	}break;

	case 2:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<YellowEnemyEntity>(playScene, spawnPos);
	}break;

	case 3:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<ArrowEnemyEntity>(playScene, spawnPos);
	}break;

	case 4:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<HealerEnemyEntity>(playScene, spawnPos);
	}break;

	case 5:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<ArmorEnemyEntity>(playScene, spawnPos);
	}break;

	case 6:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<GunnerEnemyEntity>(playScene, spawnPos);
	}break;

	case 7:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<YoroiBossEntity>(playScene, spawnPos,"assets/images/uies/HP_enemy_black.png", Vector2d(192, 192));
	}break;

	case 8:
	{
		spawnedEnemy = EnemySpawner::SpawnEnemy<SekienkiBossEntity>(playScene, spawnPos, "assets/images/uies/HP_enemy_black.png", Vector2d(192, 192));
	}break;

	default:
		std::cerr << "無効な値のため敵を生成できません。\n";
		break;
	}

	if (spawnedEnemy)
	{
		m_actors.push_back(spawnedEnemy);
	}


}

void BattleEvent::Draw()
{
	return;
}


CutInEvent::CutInEvent(Scene* scene, const std::string& filePath, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	LoadTexts(filePath);
}

CutInEvent::CutInEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	m_texts = texts;
}

CutInEvent::~CutInEvent() = default;

void CutInEvent::Init()
{
	for (auto actor : m_scene->GetActors())
	{
		actor->SetState(Actor::State::Paused);
	}

	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();

		if (texManager)
		{
			for (const std::string& text : m_texts)
			{
				auto tokens = SplitCSV(text);

				if (tokens.size() > 1 && !tokens[1].empty() && tokens[1] != "None")
				{
					m_rBossName = tokens[1];
				}

				if (tokens.size() > 2 && !tokens[2].empty() && tokens[2] != "None")
				{
					m_jBossName = tokens[2];
				}

				if (tokens.size() > 3 && !tokens[3].empty() && tokens[3] != "None")
				{
					m_displayTime = std::stof(tokens[3]);
				}

				if (tokens.size() > 4 && !tokens[4].empty() && tokens[4] != "None")
				{
					m_bossTextureId = texManager->LoadTexture(tokens[4]);
				}

				if (tokens.size() > 5 && !tokens[5].empty() && tokens[5] != "None")
				{
					m_musashiTextureId = texManager->LoadTexture(tokens[5]);
				}

				if (tokens.size() > 6 && !tokens[6].empty() && tokens[6] != "None")
				{
					m_bandTextureId = texManager->LoadTexture(tokens[6]);
				}
			}
		}

		m_isEnd = false;
	}
}

void CutInEvent::Update(float deltaTime)
{
	if(m_timer <= m_displayTime)
	{
		if ((m_timer < m_displayTime * 0.1f))
		{
			m_graphSpeed = 100;
		}
		else if ((m_timer >= m_displayTime * 0.1f) && (m_timer <= m_displayTime * 0.75f))
		{
			m_graphSpeed = 1;
			if (m_rBossNameX > 200)
			{
				m_nameSpeed = 100;
			}
			else
			{
				m_nameSpeed = 1;
			}
		}
		else if (m_timer > m_displayTime * 0.7f)
		{
			m_graphSpeed = 100;
			m_nameSpeed = 100;
		}


		m_bossX -= m_graphSpeed;
		m_musashiX += m_graphSpeed;

		m_rBossNameX -= m_nameSpeed;
		m_jBossNameX -= m_nameSpeed;

		m_timer++;
	}
	else
	{
		m_isEnd = true;
		return;
	}
}

void CutInEvent::End()
{
	for (auto actor : m_scene->GetActors())
	{
		actor->SetState(Actor::State::Active);
	}

	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();
		if (texManager)
		{
			texManager->Clear();
		}
	}
	DeleteTexts();
}

bool CutInEvent::IsEnd() const
{
	return m_isEnd;
}


void CutInEvent::Draw()
{
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if((m_bossTextureId != -1) && (m_musashiTextureId != -1))
	{
		DrawExtendGraph(m_bandX, m_bandY, m_bandX + m_bandW, m_bandY + m_bandH, m_bandTextureId, TRUE);
		DrawExtendGraph(m_bossX, m_bossY, m_bossX + m_bossW, m_bossY + m_bossH, m_bossTextureId, TRUE);
		DrawExtendGraph(m_musashiX, m_musashiY, m_musashiX + m_musashiW, m_musashiY + m_musashiH, m_musashiTextureId, TRUE);
	}

	//ChangeFont("Nexus Sans");
	SetFontSize(m_rFontSize);
	DrawString(m_rBossNameX, m_rBossNameY, m_rBossName.c_str(), m_rBossNameColor.ToDxColor());

	//ChangeFont("HGP 明朝 E");
	SetFontSize(m_jFontSize);
	DrawString(m_jBossNameX, m_jBossNameY, m_jBossName.c_str(), m_jBossNameColor.ToDxColor());
}


ClearEvent::ClearEvent(Scene* scene, const std::string& filePath, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	LoadTexts(filePath);
}

ClearEvent::ClearEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
{
	m_texts = texts;
}

ClearEvent::~ClearEvent() = default;

void ClearEvent::Init()
{
	if (m_eventManager)
	{
		Game* game = m_eventManager->GetGame();
		PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
		if (game && playScene)
		{
			float time = playScene->GetPlayTime();

			game->ChangeScene(std::make_unique<ClearScene>(game, time));
			m_isEnd = true;
		}
	}


}

void ClearEvent::Update(float deltaTime)
{
	
}

void ClearEvent::End()
{
	DeleteTexts();
}

bool ClearEvent::IsEnd() const
{
	return m_isEnd;
}

void ClearEvent::Draw()
{

}

EventTrigger::EventTrigger(Scene* scene, const Vector2d& pos, const Vector2d& size, int eventId, EventManager* eventManager)
	:BlockActor(scene)
	,m_eventId(eventId)
	,m_size(size)
	,m_isTriggered(false)
	,m_eventManager(eventManager)
{
	AddComponent<TransformComponent>();
	GetComponent<TransformComponent>()->SetPosition(pos);
}

void EventTrigger::Update(float deltaTime)
{
	if (m_isTriggered) return;


	PlayerEntity* player = nullptr;
	for (auto actor : m_scene->GetActors())
	{
		if (actor->GetType() == ActorType::Player)
		{
			player = static_cast<PlayerEntity*>(actor);
			break;
		}
	}
	if (!player) return;
	

	Vector2d playerPos = player->GetComponent<TransformComponent>()->GetPosition();
	Vector2d myPos = GetComponent<TransformComponent>()->GetPosition();

	if (playerPos.x >= myPos.x && playerPos.x <= myPos.x + m_size.x && playerPos.y >= myPos.y && playerPos.y <= myPos.y + m_size.y)
	{
		m_isTriggered = true;

		if (m_eventManager)
		{
			if (m_eventManager->IsEventTriggered(m_eventId))
			{
				SetState(Actor::State::Dead);
				return;
			}

			if (m_eventManager->IsRunning())
			{
				m_isTriggered = false; 
				return;
			}

			m_eventManager->RegisterEvent(m_eventId); //重複を避ける

			//地面をイベント開始地点とする処理
			float groundY = playerPos.y;  
			float minDistance = 99999.0f; 

			for (auto actor : m_scene->GetActors())
			{
				if (actor->GetType() == ActorType::Block && actor != this)
				{
					if (dynamic_cast<EventTrigger*>(actor) != nullptr)//イベントトリガーのBlockを見ない
					{
						continue;
					}

					auto transform = actor->GetComponent<TransformComponent>();
					if (transform)
					{
						Vector2d blockPos = transform->GetPosition();

						if (playerPos.x >= blockPos.x && playerPos.x <= blockPos.x + 64.0f)
						{
							if (blockPos.y >= playerPos.y)
							{
								float distance = blockPos.y - playerPos.y;
								if (distance < minDistance)
								{
									minDistance = distance;
									groundY = blockPos.y;
								}
							}
						}
					}
				}
			}
			Vector2d adjustedPos = { myPos.x, groundY - 100.0f};
			m_eventManager->SetTriggerPosition(adjustedPos);

			std::string filePath = "assets/events/event_" + std::to_string(m_eventId) + ".csv";
			m_eventManager->LoadEventTimeLine(filePath, adjustedPos);
		}

		SetState(Actor::State::Dead);
	}
}

BlockType EventTrigger::GetBlockType() const
{
	return BlockType::Solid;
}