#include "SoundComponent.h"
#include <algorithm>
#include "Actor.h"
#include "Scene.h"
#include "Game.h"

std::vector<SoundComponent*> SoundComponent::s_components;

SoundComponent::SoundComponent(
	Actor* owner,
	const TCHAR* filePath
)
	: Component(owner)
	, m_filePath(filePath != nullptr ? filePath : _T(""))
	, m_handle(-1)
	, m_volume(255)

{
	s_components.push_back(this);
}

SoundComponent::~SoundComponent()
{
	Release();
	s_components.erase(std::remove(s_components.begin(), s_components.end(), this), s_components.end());
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

	RefreshVolume();

	return true;
}

bool SoundComponent::Play(int playType, bool restart)
{
	if (!IsLoaded())
	{
		return false;
	}

	RefreshVolume();
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

	RefreshVolume();
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

void SoundComponent::SetCategory(SoundCategory category)
{
    m_category = category;
    RefreshVolume();
}

void SoundComponent::RefreshVolume()
{
    if (!IsLoaded() || !m_owner || !m_owner->GetScene()) return;
    Game* game = m_owner->GetScene()->GetGame();
    if (!game) return;
    const int category = m_category == SoundCategory::Bgm ? 2 : 1;
    const int effective = m_volume * game->GetVolume(0) * game->GetVolume(category) / 10000;
    ChangeVolumeSoundMem(effective, m_handle);
}

void SoundComponent::RefreshAllVolumes()
{
    for (SoundComponent* sound : s_components) sound->RefreshVolume();
}