#pragma once

#include "SoundComponent.h"
#include <cstdint>
#include <string>
#include <unordered_map>

class Game;

// Owns CSV-defined sounds. Scope 0 is always resident; one other scope can be active.
class SoundManager
{
public:
    explicit SoundManager(Game* game);
    ~SoundManager();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    bool LoadCsv(const std::string& csvPath);
    bool SetScope(int scope);
    bool Play(const std::string& soundId, SoundCategory category = SoundCategory::Effect, bool restart = true);
    void Stop(const std::string& soundId);
    bool IsPlaying(const std::string& soundId) const;
    void Update(float deltaTime);
    void RefreshVolumes();
    void UnloadAll();

    int GetScope() const { return m_scope; }
    static void RefreshActiveVolumes();

private:
    struct Sound
    {
        std::string filePath;
        int scope = 0;
        int volume = 255;
        int pan = 0;
        std::int64_t startMs = 0;
        std::int64_t endMs = 0;
        bool loop = false;
        int loopFadeFrames = 0;
        SoundCategory category = SoundCategory::Effect;
        int handles[2] = { -1, -1 };
        int active = 0;
        bool playing = false;
        bool fading = false;
        float fade = 0.0f;
        double elapsedMs = 0.0;
    };

    bool Load(Sound& sound);
    void Unload(Sound& sound);
    void Stop(Sound& sound);
    bool Start(Sound& sound, int slot);
    void ApplyVolume(Sound& sound);

    Game* m_game;
    int m_scope = -1;
    std::unordered_map<std::string, Sound> m_sounds;
    static SoundManager* s_active;
};

