#pragma once

#include "Component.h"
#include <DxLib.h>
#include <string>
#include <vector>

enum class SoundCategory { Effect, Bgm };

class SoundComponent : public Component
{
public:
	explicit SoundComponent(
		Actor* owner,
		const TCHAR* filePath
	);
	~SoundComponent() override;

	SoundComponent(const SoundComponent&) = delete;
	SoundComponent& operator=(const SoundComponent&) = delete;

	bool Init() override;

	bool Play(
		int playType = DX_PLAYTYPE_BACK,
		bool restart = true
	);
	void Stop();
	bool IsPlaying() const;

	void SetVolume(int volume);
	void SetCategory(SoundCategory category);
	void RefreshVolume();
	static void RefreshAllVolumes();
	void Release();
	bool IsLoaded() const;

private:
	std::basic_string<TCHAR> m_filePath;
	int m_handle;
	int m_volume;
	SoundCategory m_category = SoundCategory::Effect;
	static std::vector<SoundComponent*> s_components;
};
