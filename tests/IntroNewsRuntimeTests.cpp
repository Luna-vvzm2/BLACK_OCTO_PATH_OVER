// Runs the real dialogue -> newspaper transition and exports its rendered frame.
#include "Game.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "EventManager.h"
#include "EventTexture.h"
#include "SaveManager.h"
#include "SaveData.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    bool failed = false;
    bool completed = false;

    bool Expect(Game* game, bool condition, const char* label)
    {
        std::cout << (condition ? "PASS " : "FAIL ") << label << std::endl;
        if (!condition) { failed = true; game->RequestQuit(); }
        return condition;
    }

    class NewsProbe : public PlayScene
    {
    public:
        explicit NewsProbe(Game* game) : PlayScene(game) {}
        bool Init() override
        {
            if (!PlayScene::Init()) return false;
            for (auto* actor : GetActors())
                if (dynamic_cast<EventTrigger*>(actor)) actor->SetState(Actor::State::Dead);
            timeline = std::make_unique<EventManager>(this, &textures);
            return true;
        }
        void Update(float dt) override
        {
            if (completed || failed) return;
            PlayScene::Update(dt);
            elapsed += dt;
            if (elapsed > 12.0f)
            {
                Expect(m_game, false, "intro news transition completes before timeout");
                return;
            }
            if (!started && GetPlayer()->OnGround())
            {
                timeline->LoadEventTimeLine("intro_news_fixture.csv", Vector2d::Zero());
                if (!Expect(m_game, timeline->IsRunning() && !IsNewsOpen(),
                    "dialogue starts before newspaper")) return;
                started = true;
            }
            if (started) timeline->Update(dt);
            if (IsNewsOpen()) shown += dt;
            if (previewSaved)
            {
                if (!Expect(m_game, IsNewsOpen() && timeline->IsRunning(),
                    "newspaper keeps the event active")) return;
                CloseNews();
                timeline->Update(dt);
                if (!Expect(m_game, !IsNewsOpen() && !timeline->IsRunning(),
                    "closing newspaper completes the event")) return;
                completed = true;
                m_game->RequestQuit();
            }
        }
        void Draw() override
        {
            PlayScene::Draw();
            if (started) timeline->Draw();
            if (shown >= 0.7f && !previewSaved)
            {
                previewSaved = SaveDrawScreenToPNG(0, 0, 1280, 720, "intro_news_preview.png") == 0;
                Expect(m_game, previewSaved, "newspaper frame exported");
            }
        }
    private:
        EventTexture textures;
        std::unique_ptr<EventManager> timeline;
        float elapsed = 0.0f;
        float shown = 0.0f;
        bool started = false;
        bool previewSaved = false;
    };
}

int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    std::filesystem::current_path(argv[1]);
    std::filesystem::create_directories("save");
    SaveData save;
    save.currentStage = 0;
    if (!SaveManager::Save(save)) return 2;
    {
        std::ifstream intro("assets/events/event_101.csv");
        std::string line;
        int talks = 0, newspapers = 0;
        while (std::getline(intro, line))
        {
            if (line.rfind("TALK,", 0) == 0)
            {
                if (newspapers != 0) return 2;
                ++talks;
            }
            if (line.rfind("NEWS,1,", 0) == 0)
            {
                if (talks == 0) return 2;
                ++newspapers;
            }
        }
        if (talks == 0 || newspapers != 1) return 2;
    }
    {
        std::ofstream fixture("intro_news_fixture.csv");
        fixture << "TALK,Reporter,0.05,None,,,Intro dialogue\nNEWS,1,,,,,\n";
    }
    SetWindowVisibleFlag(FALSE);
    SetAlwaysRunFlag(TRUE);
    SetUseDirectInputFlag(FALSE);
    Game game;
    if (!game.Init("Intro news runtime regression", 1280, 720, 32, "Arial", "Arial"))
    {
        game.End();
        return 2;
    }
    game.ChangeScene(std::make_unique<NewsProbe>(&game));
    game.Run();
    game.End();
    if (failed || !completed) return 1;
    std::cout << "Intro dialogue and newspaper runtime tests passed.\n";
}
