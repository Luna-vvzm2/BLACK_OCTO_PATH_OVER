#include "SoundComponent.h"
#include <algorithm>
#include "Actor.h"
#include "Scene.h"
#include "Game.h"
#include "SoundManager.h"

std::vector<SoundComponent*> SoundComponent::s_components;
int SoundComponent::s_voiceVolume = 100;

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
		return true;
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

bool SoundComponent::Play(const std::string& soundId, bool restart)
{
	if (!m_owner || !m_owner->GetScene()) return false;
	Game* game = m_owner->GetScene()->GetGame();
	if (!game || !game->GetSoundManager().Play(soundId, m_category, restart)) return false;
	m_lastSoundId = soundId;
	return true;
}

void SoundComponent::Stop()
{
	if (IsLoaded())
	{
		StopSoundMem(m_handle);
	}
	if (!m_lastSoundId.empty()) Stop(m_lastSoundId);
}

void SoundComponent::Stop(const std::string& soundId)
{
	if (!m_owner || !m_owner->GetScene()) return;
	Game* game = m_owner->GetScene()->GetGame();
	if (game) game->GetSoundManager().Stop(soundId);
}

bool SoundComponent::IsPlaying() const
{
	return (IsLoaded() && CheckSoundMem(m_handle) == 1) ||
		(!m_lastSoundId.empty() && IsPlaying(m_lastSoundId));
}

bool SoundComponent::IsPlaying(const std::string& soundId) const
{
	if (!m_owner || !m_owner->GetScene()) return false;
	Game* game = m_owner->GetScene()->GetGame();
	return game && game->GetSoundManager().IsPlaying(soundId);
}

void SoundComponent::SetVolume(int volume)
{
	m_volume = std::clamp(volume, 0, 255);

	RefreshVolume();
}

void SoundComponent::Release()
{
	if (!m_lastSoundId.empty())
	{
		Stop(m_lastSoundId);
		m_lastSoundId.clear();
	}
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
    const int categoryVolume = m_category == SoundCategory::Voice
        ? s_voiceVolume
        : game->GetVolume(m_category == SoundCategory::Bgm ? 2 : 1);
    const int effective = m_volume * game->GetVolume(0) * categoryVolume / 10000;
    ChangeVolumeSoundMem(effective, m_handle);
}

void SoundComponent::RefreshAllVolumes()
{
    for (SoundComponent* sound : s_components) sound->RefreshVolume();
	SoundManager::RefreshActiveVolumes();
}

int SoundComponent::GetVoiceVolume()
{
    return s_voiceVolume;
}

void SoundComponent::AdjustVoiceVolume(int amount)
{
    s_voiceVolume = std::clamp(s_voiceVolume + amount, 0, 100);
    RefreshAllVolumes();
}
