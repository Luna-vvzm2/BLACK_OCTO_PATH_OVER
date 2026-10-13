#include "SoundManager.h"
#include "Game.h"

#include <DxLib.h>
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace
{
    std::filesystem::path ResolveCsvPath(const std::string& csvPath)
    {
        const auto requested = std::filesystem::u8path(csvPath);
        if (std::filesystem::exists(requested)) return std::filesystem::absolute(requested);

        wchar_t modulePath[MAX_PATH] = {};
        if (GetModuleFileNameW(nullptr, modulePath, MAX_PATH) == 0) return requested;
        auto directory = std::filesystem::path(modulePath).parent_path();
        for (int depth = 0; depth < 4; ++depth)
        {
            const auto candidate = directory / requested;
            if (std::filesystem::exists(candidate)) return candidate;
            if (directory == directory.parent_path()) break;
            directory = directory.parent_path();
        }
        return requested;
    }

    std::vector<std::string> SplitCsv(const std::string& line)
    {
        std::vector<std::string> fields;
        std::string field;
        bool quoted = false;
        for (size_t i = 0; i < line.size(); ++i)
        {
            const char c = line[i];
            if (c == '"')
            {
                if (quoted && i + 1 < line.size() && line[i + 1] == '"')
                {
                    field += '"';
                    ++i;
                }
                else quoted = !quoted;
            }
            else if (c == ',' && !quoted)
            {
                fields.push_back(field);
                field.clear();
            }
            else if (c != '\r') field += c;
        }
        fields.push_back(field);
        return fields;
    }

    bool ParseInt(const std::string& text, int& value)
    {
        try
        {
            size_t used = 0;
            const long parsed = std::stol(text, &used);
            if (used != text.size() || parsed < (std::numeric_limits<int>::min)() ||
                parsed > (std::numeric_limits<int>::max)()) return false;
            value = static_cast<int>(parsed);
            return true;
        }
        catch (...) { return false; }
    }

    bool ParseTime(const std::string& text, std::int64_t& milliseconds)
    {
        if (text.empty()) return false;
        if (text.find(':') == std::string::npos)
        {
            int frames = 0;
            if (!ParseInt(text, frames) || frames < 0) return false;
            milliseconds = static_cast<std::int64_t>(frames) * 1000 / 60;
            return true;
        }
        std::istringstream input(text);
        int hours = 0, minutes = 0, seconds = 0, frames = 0;
        char colon1 = 0, colon2 = 0, colon3 = 0;
        if (!(input >> hours >> colon1 >> minutes >> colon2 >> seconds >> colon3 >> frames) ||
            input.peek() != EOF || colon1 != ':' || colon2 != ':' || colon3 != ':' ||
            hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60 ||
            frames < 0 || frames >= 60) return false;
        const std::int64_t totalFrames =
            ((static_cast<std::int64_t>(hours) * 60 + minutes) * 60 + seconds) * 60 + frames;
        milliseconds = totalFrames * 1000 / 60;
        return true;
    }

    std::string ToLocalPath(const std::string& utf8)
    {
        const int wideSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            utf8.c_str(), -1, nullptr, 0);
        if (wideSize == 0) return {};
        std::wstring wide(static_cast<size_t>(wideSize), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1,
            &wide[0], wideSize);
        const int localSize = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1,
            nullptr, 0, nullptr, nullptr);
        if (localSize == 0) return {};
        std::string local(static_cast<size_t>(localSize), '\0');
        WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, &local[0], localSize,
            nullptr, nullptr);
        local.pop_back();
        return local;
    }

    std::int64_t FadeDurationMs(int frames, std::int64_t start, std::int64_t end)
    {
        const std::int64_t length = end - start;
        if (frames <= 0 || length < 4) return 0;
        const std::int64_t requested = static_cast<std::int64_t>(frames) * 1000 / 60;
        return requested < length / 2 ? requested : length / 2;
    }
}

SoundManager* SoundManager::s_active = nullptr;

SoundManager::SoundManager(Game* game) : m_game(game)
{
    s_active = this;
}

SoundManager::~SoundManager()
{
    UnloadAll();
    if (s_active == this) s_active = nullptr;
}

bool SoundManager::LoadCsv(const std::string& csvPath)
{
    const auto resolvedCsvPath = ResolveCsvPath(csvPath);
    std::ifstream file(resolvedCsvPath, std::ios::binary);
    if (!file)
    {
        std::cerr << "SoundManager: cannot open " << resolvedCsvPath.u8string() << '\n';
        return false;
    }

    const auto csvDirectory = resolvedCsvPath.parent_path();
    const auto assetRoot = csvDirectory.filename() == L"SoundAssets"
        ? csvDirectory.parent_path() : csvDirectory;

    std::string line;
    if (!std::getline(file, line)) return false;
    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
    const auto header = SplitCsv(line);
    const char* required[] = { "SoundId", "FilePath", "LoadScope", "Volume", "Pan",
        "SourceStart", "SourceEnd", "Loop", "LoopFadeFrames" };
    std::vector<size_t> columns;
    for (const char* name : required)
    {
        auto found = std::find(header.begin(), header.end(), name);
        if (found == header.end() && std::string(name) == "SourceStart")
            found = std::find(header.begin(), header.end(), "SourceStartFrame");
        if (found == header.end() && std::string(name) == "SourceEnd")
            found = std::find(header.begin(), header.end(), "SourceEndFrame");
        if (found == header.end())
        {
            std::cerr << "SoundManager: missing column " << name << '\n';
            return false;
        }
        columns.push_back(static_cast<size_t>(found - header.begin()));
    }

    std::unordered_map<std::string, Sound> parsed;
    int rowNumber = 1;
    while (std::getline(file, line))
    {
        ++rowNumber;
        if (line.empty() || line == "\r") continue;
        const auto values = SplitCsv(line);
        if (values.size() < header.size()) return false;
        const auto& id = values[columns[0]];
        Sound sound;
        const std::string& relativeFilePath = values[columns[1]];
        const std::string& loop = values[columns[7]];
        sound.loop = loop == "true" || loop == "TRUE" || loop == "1";
        if (id.empty() || relativeFilePath.empty() ||
            !ParseInt(values[columns[2]], sound.scope) || sound.scope < 0 ||
            !ParseInt(values[columns[3]], sound.volume) || sound.volume < 0 || sound.volume > 255 ||
            !ParseInt(values[columns[4]], sound.pan) || sound.pan < -255 || sound.pan > 255 ||
            !ParseTime(values[columns[5]], sound.startMs) ||
            !ParseTime(values[columns[6]], sound.endMs) ||
            !ParseInt(values[columns[8]], sound.loopFadeFrames) || sound.loopFadeFrames < 0 ||
            (loop != "true" && loop != "false" && loop != "TRUE" &&
             loop != "FALSE" && loop != "0" && loop != "1") ||
            relativeFilePath.find("..") != std::string::npos ||
            relativeFilePath.find(':') != std::string::npos ||
            relativeFilePath[0] == '/' || relativeFilePath[0] == '\\')
        {
            std::cerr << "SoundManager: invalid CSV row " << rowNumber << '\n';
            return false;
        }
        sound.filePath = (assetRoot / std::filesystem::u8path(relativeFilePath)).u8string();
        if (!parsed.emplace(id, std::move(sound)).second)
        {
            std::cerr << "SoundManager: duplicate SoundId on CSV row " << rowNumber << '\n';
            return false;
        }
    }

    UnloadAll();
    m_sounds = std::move(parsed);
    m_scope = -1;
    return true;
}

bool SoundManager::Load(Sound& sound)
{
    if (sound.handles[0] != -1) return true;
    const std::string localPath = ToLocalPath(sound.filePath);
    if (localPath.empty()) return false;
    sound.handles[0] = LoadSoundMem(localPath.c_str());
    if (sound.handles[0] == -1) return false;
    const auto duration = GetSoundTotalTime(sound.handles[0]);
    if (duration <= 0 || sound.startMs >= duration)
    {
        Unload(sound);
        return false;
    }
    if (sound.endMs <= 0 || sound.endMs > duration) sound.endMs = duration;
    if (sound.endMs <= sound.startMs)
    {
        Unload(sound);
        return false;
    }
    const auto fadeMs = FadeDurationMs(sound.loopFadeFrames, sound.startMs, sound.endMs);
    if (sound.loop && fadeMs > 0)
    {
        sound.handles[1] = DuplicateSoundMem(sound.handles[0]);
        if (sound.handles[1] == -1)
            sound.handles[1] = LoadSoundMem(localPath.c_str());
        if (sound.handles[1] == -1)
        {
            Unload(sound);
            return false;
        }
    }
    for (const int handle : sound.handles)
        if (handle != -1) ChangePanSoundMem(sound.pan, handle);
    ApplyVolume(sound);
    return true;
}

void SoundManager::Unload(Sound& sound)
{
    Stop(sound);
    for (int& handle : sound.handles)
    {
        if (handle != -1) DeleteSoundMem(handle);
        handle = -1;
    }
}

bool SoundManager::SetScope(int scope)
{
    if (scope < 0) return false;
    if (scope == m_scope) return true;
    for (auto& pair : m_sounds)
    {
        Sound& sound = pair.second;
        if (sound.scope != 0 && sound.scope != scope) Unload(sound);
    }
    bool success = true;
    for (auto& pair : m_sounds)
    {
        Sound& sound = pair.second;
        if ((sound.scope == 0 || sound.scope == scope) && !Load(sound))
        {
            std::cerr << "SoundManager: cannot load ID " << pair.first << '\n';
            success = false;
        }
    }
    m_scope = scope;
    return success;
}

bool SoundManager::Start(Sound& sound, int slot)
{
    const int handle = sound.handles[slot];
    if (handle == -1) return false;
    StopSoundMem(handle);
    if (SetSoundCurrentTime(sound.startMs, handle) == -1) return false;
    return PlaySoundMem(handle, DX_PLAYTYPE_BACK, FALSE) == 0;
}

bool SoundManager::Play(const std::string& soundId, SoundCategory category, bool restart)
{
    auto found = m_sounds.find(soundId);
    if (found == m_sounds.end() || found->second.handles[0] == -1) return false;
    Sound& sound = found->second;
    if (sound.playing && !restart) return true;
    Stop(sound);
    sound.category = category;
    sound.active = 0;
    sound.elapsedMs = static_cast<double>(sound.startMs);
    sound.playing = Start(sound, 0);
    ApplyVolume(sound);
    return sound.playing;
}

void SoundManager::Stop(Sound& sound)
{
    for (const int handle : sound.handles)
        if (handle != -1) StopSoundMem(handle);
    sound.playing = false;
    sound.fading = false;
    sound.fade = 0.0f;
}

void SoundManager::Stop(const std::string& soundId)
{
    auto found = m_sounds.find(soundId);
    if (found != m_sounds.end()) Stop(found->second);
}

bool SoundManager::IsPlaying(const std::string& soundId) const
{
    const auto found = m_sounds.find(soundId);
    return found != m_sounds.end() && found->second.playing;
}

void SoundManager::Update(float deltaTime)
{
    for (auto& pair : m_sounds)
    {
        Sound& sound = pair.second;
        if (!sound.playing) continue;
        const int active = sound.handles[sound.active];
        const bool stillPlaying = CheckSoundMem(active) == 1;
        const auto position = stillPlaying ? GetSoundCurrentTime(active) : -1;
        sound.elapsedMs += static_cast<double>(deltaTime) * 1000.0;
        const double current = position >= 0 ? static_cast<double>(position) : sound.elapsedMs;
        if (!sound.loop)
        {
            if (!stillPlaying || current >= sound.endMs) Stop(sound);
            continue;
        }

        const auto fadeMs = FadeDurationMs(sound.loopFadeFrames, sound.startMs, sound.endMs);
        const bool canFade = sound.handles[1] != -1 && fadeMs > 0;
        if (!canFade)
        {
            if (!stillPlaying || current >= sound.endMs)
            {
                StopSoundMem(active);
                sound.playing = Start(sound, sound.active);
                sound.elapsedMs = static_cast<double>(sound.startMs);
            }
            continue;
        }

        const double fadeStart = static_cast<double>(sound.endMs - fadeMs);
        if (!sound.fading && (current >= fadeStart || !stillPlaying))
        {
            const int alternate = 1 - sound.active;
            if (!Start(sound, alternate)) { Stop(sound); continue; }
            sound.fading = true;
            sound.fade = 0.0f;
            ApplyVolume(sound);
        }
        if (sound.fading)
        {
            sound.fade = std::clamp(static_cast<float>((current - fadeStart) / fadeMs), 0.0f, 1.0f);
            if (!stillPlaying) sound.fade = 1.0f;
            ApplyVolume(sound);
            if (sound.fade >= 1.0f)
            {
                StopSoundMem(active);
                sound.active = 1 - sound.active;
                sound.fading = false;
                sound.fade = 0.0f;
                sound.elapsedMs = static_cast<double>(sound.startMs + fadeMs);
                ApplyVolume(sound);
            }
        }
    }
}

void SoundManager::ApplyVolume(Sound& sound)
{
    if (!m_game) return;
    const int channel = sound.category == SoundCategory::Bgm ? 2 : 1;
    const int categoryVolume = sound.category == SoundCategory::Voice
        ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(channel);
    const int volume = sound.volume * m_game->GetVolume(0) * categoryVolume / 10000;
    for (int slot = 0; slot < 2; ++slot)
    {
        if (sound.handles[slot] == -1) continue;
        const float factor = sound.fading
            ? (slot == sound.active ? 1.0f - sound.fade : sound.fade)
            : (slot == sound.active ? 1.0f : 0.0f);
        ChangeVolumeSoundMem(std::clamp(static_cast<int>(std::lround(volume * factor)), 0, 255),
            sound.handles[slot]);
    }
}

void SoundManager::RefreshVolumes()
{
    for (auto& pair : m_sounds) ApplyVolume(pair.second);
}

void SoundManager::RefreshActiveVolumes()
{
    if (s_active) s_active->RefreshVolumes();
}

void SoundManager::UnloadAll()
{
    for (auto& pair : m_sounds) Unload(pair.second);
    m_scope = -1;
}

