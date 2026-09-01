#include "SoundComponent.h"
#include <algorithm>

SoundComponent::SoundComponent(
	Actor* owner,
	const TCHAR* filePath
)
	: Component(owner)
	, m_filePath(filePath != nullptr ? filePath : _T(""))
	, m_handle(-1)
	, m_volume(255)

{
}

SoundComponent::~SoundComponent()
{
	Release();
}

bool SoundComponent::Init()
{
	if (m_filePath.empty())
	{
		return false;
	}

	m_handle = LoadSoundMem(m_filePath.c_str());
	if (!IsLoaded())
	{
		return false;
	}

	if (ChangeVolumeSoundMem(m_volume, m_handle) == -1)
	{
		Release();
		return false;
	}

	return true;
}

bool SoundComponent::Play(int playType, bool restart)
{
	if (!IsLoaded())
	{
		return false;
	}

	return PlaySoundMem(
		m_handle,
		playType,
		restart ? TRUE : FALSE
	) == 0;
}

void SoundComponent::Stop()
{
	if (IsLoaded())
	{
		StopSoundMem(m_handle);
	}
}

bool SoundComponent::IsPlaying() const
{
	return IsLoaded() && CheckSoundMem(m_handle) == 1;
}

void SoundComponent::SetVolume(int volume)
{
	m_volume = std::clamp(volume, 0, 255);

	if (IsLoaded())
	{
		ChangeVolumeSoundMem(m_volume, m_handle);
	}
}

void SoundComponent::Release()
{
	if (!IsLoaded())
	{
		return;
	}

	StopSoundMem(m_handle);
	DeleteSoundMem(m_handle);
	m_handle = -1;
}

bool SoundComponent::IsLoaded() const
{
	return m_handle != -1;
}
