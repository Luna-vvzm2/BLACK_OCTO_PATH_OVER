// Link against the game's Debug objects, excluding Main.obj.
// Run in an isolated working directory containing assets/ and save/.
#include "Game.h"
#include "AddEnemySpawner.h"
#include "EventTexture.h"
#include "EventManager.h"
#include "PlayScene.h"
#include "TitleScene.h"
#include "SaveData.h"
#include "SaveManager.h"
#include <filesystem>
#include <iostream>
#include <memory>

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

    class ResumeOverlay : public Scene
    {
    public:
        explicit ResumeOverlay(Game* game) : Scene(game) {}
        bool Init() override { m_type = Type::StageSelect; return m_isRunning = true; }
        void Draw() override {}
        void Update(float dt) override
        {
            elapsed += dt;
            if (elapsed > 0.5f) m_game->PopScene();
        }
    private:
        float elapsed = 0.0f;
    };

    class ExitProbe : public TitleScene
    {
    public:
        explicit ExitProbe(Game* game) : TitleScene(game) {}
        bool Init() override
        {
            const bool initialized = TitleScene::Init();
            const auto& sound = m_game->GetSoundManager();
            Expect(m_game, !sound.IsPlaying("Stage_Common_BGM") && !sound.IsPlaying("Stage3_BGM"),
                "leaving gameplay stops stage BGM");
            completed = !failed;
            m_game->RequestQuit();
            return initialized;
        }
    };

    class PlayProbe : public PlayScene
    {
    public:
        explicit PlayProbe(Game* game) : PlayScene(game) {}
        bool Init() override
        {
            const bool initialized = PlayScene::Init();
            Expect(m_game, initialized, "play scene initialization");
            return initialized;
        }
        void Update(float dt) override
        {
            PlayScene::Update(dt);
            elapsed += dt;
            if (elapsed < 1.0f) return;
            elapsed = 0.0f;
            const auto& sound = m_game->GetSoundManager();
            if (step == 0)
            {
                if (!Expect(m_game, sound.IsPlaying("Stage_Common_BGM") && !sound.IsPlaying("Stage3_BGM"),
                    "stage 1 common BGM plays")) return;
                ChangeStage(1, 0);
            }
            else if (step == 1)
            {
                if (!Expect(m_game, GetStageIndex() == 1 && sound.IsPlaying("Stage_Common_BGM"),
                    "stage 1 to 2 keeps common BGM")) return;
                ChangeStage(2, 0);
            }
            else if (step == 2)
            {
                if (!Expect(m_game, sound.IsPlaying("Stage3_BGM") && !sound.IsPlaying("Stage_Common_BGM"),
                    "stage 2 to 3 switches BGM")) return;
                m_game->PushScene(std::make_unique<ResumeOverlay>(m_game));
            }
            else if (step == 3)
            {
                if (!Expect(m_game, sound.GetScope() == 4 && sound.IsPlaying("Stage3_BGM"),
                    "returning from overlay restores stage 3 BGM")) return;
                ChangeStage(0, 0);
            }
            else if (step == 4)
            {
                if (!Expect(m_game, sound.IsPlaying("Stage_Common_BGM") && !sound.IsPlaying("Stage3_BGM"),
                    "stage 3 to 1 restores common BGM")) return;
                m_game->ChangeScene(std::make_unique<ExitProbe>(m_game));
            }
            ++step;
        }
    private:
        int step = 0;
        float elapsed = 0.0f;
    };
}

int main(int argc, char** argv)
{
    if (argc != 2) { std::cerr << "Pass an isolated runtime directory.\n"; return 2; }
    std::filesystem::current_path(argv[1]);
    std::filesystem::create_directories("save");
    SaveData data;
    data.currentStage = 0;
    data.stageClear.fill(true);
    if (!SaveManager::Save(data)) return 2;
    SetWindowVisibleFlag(FALSE);
    SetAlwaysRunFlag(TRUE);
    SetUseDirectInputFlag(FALSE);
    Game game;
    if (!game.Init("BGM runtime regression", 1280, 720, 32, "Arial", "Arial"))
    {
        std::cerr << "DxLib/game initialization failed.\n";
        game.End();
        return 2;
    }
    game.ChangeScene(std::make_unique<PlayProbe>(&game));
    game.Run();
    game.End();
    if (failed || !completed) return 1;
    std::cout << "BGM runtime integration tests passed.\n";
}
