#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "Color.h"
#include "BlockActor.h"
#include "Vector2d.h"

class Scene;
class Game;
class Input;
class EventBase;
class EventTexture;
class Actor;

class EventManager
{
public:
	EventManager(Scene* scene, EventTexture* eventTexture); //コンストラクタ
	~EventManager(); //デストラクタ

	void Init(std::unique_ptr<EventBase> event); //イベント開始
	void Update(float deltaTIme); //更新
	void End(); //イベント終了

	bool IsRunning() const; //イベントが実行中か
	bool IsBattleEvent() const; //戦闘専用
	bool IsEventTriggered(int eventId) const;
	void RegisterEvent(int eventId);

	void LoadEventTimeLine(const std::string& filePath, const Vector2d& triggerPos); //複数の連続するイベントを読み込む
	void SetTriggerPosition(const Vector2d& pos) { m_triggerPos = pos; }
	const Vector2d& GetTriggerPosition() const { return m_triggerPos; }
	Game* GetGame() const;

	void Draw();

	EventTexture* GetEventTexture() const;

private:
	Scene* m_scene{ nullptr }; //イベントを管理するシーン
	EventTexture* m_eventTexture{ nullptr }; //イベントの画像を管理
	//std::unique_ptr<EventBase> m_currentEvent{ nullptr }; //実行中のイベント
	std::vector<std::unique_ptr<EventBase>> m_eventQueue; //イベントのキュー
	size_t m_currentEventIndex{ 0 };
	std::set<int> m_triggeredEvents;

	Vector2d m_triggerPos{ 0.0f, 0.0f };
};

enum class EventType
{
	Talk,
	Battle,
	CutIn,
	Clear
};

class EventBase
{
public:

	EventBase(); //コンストラクタ
	virtual ~EventBase(); //デストラクタ

	virtual EventType GetType() const = 0;

	virtual void Init() = 0; //イベント開始時の処理
	virtual void Update(float deltaTime) = 0; //更新
	virtual void End() = 0; //終了

	virtual bool IsEnd() const = 0;

	virtual void Draw() = 0;

protected:
	void LoadTexts(const std::string& filePath); //テキストファイルの読み込み
	void DeleteTexts(); //テキストデータの解放
	std::vector<std::string> m_texts; //表示テキスト
};

class TalkEvent : public EventBase
{
public:
	TalkEvent(Scene* scene, const std::string& filePath, EventManager* eventManager); //コンストラクタ
	TalkEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager);
	~TalkEvent() override; //デストラクタ

	EventType GetType() const override { return EventType::Talk; }

	void Init() override;
	void Update(float deltaTime) override;
	void End() override;

	bool IsEnd() const override;

	enum class ActorPosition
	{
		None,
		Left,
		Right
	};

private:
	Scene* m_scene{ nullptr };
	EventManager* m_eventManager{ nullptr };
	int m_currentLine{ 0 }; //表示中のテキストインデックス
	int m_skipTimer{ 90 }; //スキップの長押しタイマー
	bool m_isEnd{ false }; //イベント終了確認

	std::string m_talkerName{ "" };
	std::string m_talkText{ "" };
	bool m_isTextTimerActive{ false }; //テキスト表示タイマーが有効か
	float m_textTimer{ 0.0f }; //テキスト表示タイマー
	ActorPosition m_talkerPosition{ ActorPosition::None };
	int m_actorTextureId{ -1 }; //立ち絵のid
	int m_actorW{ 480 };
	int m_actorH{ 510 };

	int m_boxX{ 280 };
	int m_boxY{ 510 };
	int m_boxW{ 720 };
	int m_boxH{ 160 };

	int m_fontSize{ 20 };
	int m_nameX{ 0 };
	int m_nameY{ m_boxY - 30 };
	int m_nameX2{ m_boxW + 230 };

	Color m_textColor{ 255, 255, 255, 255 };
	Color m_boxColor{ 0, 0, 0, 180 };
	Color m_nameColor{ 250, 190, 20, 255 };

	void ShowText(); //テキストを描画
	void NextText(); //次のテキストへ

	void Draw() override;
	
	//void PlayVoice();

}; 


class BattleEvent : public EventBase
{
public:
	BattleEvent(Scene* scene, const std::string& filePath, EventManager* eventManager);
	BattleEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager);
	~BattleEvent() override;

	EventType GetType() const override { return EventType::Battle; }

	void Init() override;
	void Update(float deltaTime) override;
	void End() override;

	bool IsEnd() const override;

	void SetTrigger(const Vector2d& pos) { m_triggerPos = pos; }

private:
	Scene* m_scene{ nullptr };
	EventManager* m_eventManager{ nullptr };

    bool m_isEnd{ false };
	bool m_isSpawned{ false };
	bool m_isAreaSet{ false };

	int m_enemyType{ 0 };
	Vector2d m_enemyPos{ 0.0f, -88.0f };
	Vector2d m_triggerPos{ 0.0f, 0.0f };

	float m_areaXMin{ -64.0f };
	float m_areaXMax{ 1216.0f };

	std::vector<Actor*> m_actors;
	bool m_isPlayerInsideArea{ false };

	void EnemySpawn();
	void Draw() override;
};


class CutInEvent : public EventBase
{
public:
	CutInEvent(Scene* scene, const std::string& filePath, EventManager* eventManager);
	CutInEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager);
	~CutInEvent() override;

	EventType GetType() const override { return EventType::CutIn; }

	void Init() override;
	void Update(float deltaTime) override;
	void End() override;

	bool IsEnd() const override;

private:
	Scene* m_scene{ nullptr };
	EventManager* m_eventManager{ nullptr };

	float m_timer{ 0.0f };
	float m_displayTime{ 180.0f }; //カットインの表示時間
	bool m_isEnd{ false };

	int m_graphSpeed{ 10 }; //画像の移動速度
	int m_nameSpeed{ 0 }; //テキストの移動速度

	std::string m_rBossName{ "" }; //ローマ字名
	std::string m_jBossName{ "" }; //日本語名

	int m_bandTextureId{ -1 };
	int m_bossTextureId{ -1 };
	int m_musashiTextureId{ -1 };

	int m_bandX{ 0 };
	int m_bandY{ 0 };
	int m_bandW{ 1280 };
	int m_bandH{ 720 };

	int m_bossX{ 2300 };
	int m_bossY{ 0 };
	int m_bossW{ 362 };
	int m_bossH{ 720 };

	int m_musashiX{ -1400 };
	int m_musashiY{ 385 };
	int m_musashiW{ 378 };
	int m_musashiH{ 335 };

	int m_rBossNameX{ 1280 };
	int m_rBossNameY{ 320 };

	int m_jBossNameX{ 1430 };
	int m_jBossNameY{ 380 };

	int m_rFontSize{ 96 };
	int m_jFontSize{ 60 };

	Color m_rBossNameColor{ 255, 255, 255, 255 };
	Color m_jBossNameColor{ 230, 7, 7, 255 };

	void Draw() override;
};

class ClearEvent : public EventBase
{
public:
	ClearEvent(Scene* scene, const std::string& filePath, EventManager* eventManager);
	ClearEvent(Scene* scene, const std::vector<std::string>& texts, EventManager* eventManager);
	~ClearEvent() override;

	EventType GetType() const override { return EventType::Clear; }

	void Init() override;
	void Update(float deltaTime) override;
	void End() override;

	bool IsEnd() const override;

private:
	Scene* m_scene{ nullptr };
	EventManager* m_eventManager{ nullptr };

	bool m_isEnd{ false };
	void Draw() override;
};

class EventTrigger : public BlockActor
{
public:
	EventTrigger(Scene* scene, const Vector2d& pos, const Vector2d& size, int eventId, EventManager* eventManager);

	void Update(float deltaTime) override;
	void Draw() override {}

	BlockType GetBlockType() const override;

private:
	int m_eventId;
	Vector2d m_size;
	bool m_isTriggered;
	EventManager* m_eventManager;

protected:
	std::string GetTexturePath() const override { return ""; }

};

