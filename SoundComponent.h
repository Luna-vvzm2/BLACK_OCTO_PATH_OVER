#pragma once

#include "Component.h"
#include <DxLib.h>
#include <string>
#include <vector>

// Sound.csv Category: 0 = BGM, 1 = SE, 2 = Voice.
enum class SoundCategory { Bgm = 0, Effect = 1, Voice = 2 };

class SoundComponent : public Component
{
public:
	explicit SoundComponent(
		Actor* owner,
		const TCHAR* filePath = nullptr
	);
	~SoundComponent() override;

	SoundComponent(const SoundComponent&) = delete;
	SoundComponent& operator=(const SoundComponent&) = delete;

	bool Init() override;

	bool Play(
		int playType = DX_PLAYTYPE_BACK,
		bool restart = true
	);
	bool Play(const std::string& soundId, bool restart = true);
	void Stop();
	void Stop(const std::string& soundId);
	bool IsPlaying() const;
	bool IsPlaying(const std::string& soundId) const;

	void SetVolume(int volume);
	void SetCategory(SoundCategory category);
	void RefreshVolume();
	static void RefreshAllVolumes();
	static int GetVoiceVolume();
	static void AdjustVoiceVolume(int amount);
	void Release();
	bool IsLoaded() const;

private:
	std::basic_string<TCHAR> m_filePath;
	int m_handle;
	int m_volume;
	SoundCategory m_category = SoundCategory::Effect;
	std::string m_lastSoundId;
	static std::vector<SoundComponent*> s_components;
	static int s_voiceVolume;
};
