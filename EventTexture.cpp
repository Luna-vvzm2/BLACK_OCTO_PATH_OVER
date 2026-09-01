#include "EventTexture.h"
#include "DxLib.h"
#include <iostream>

EventTexture::EventTexture() = default;
EventTexture::~EventTexture()
{
	Clear();
}

int EventTexture::LoadTexture(const std::string& path)
{
	auto it = m_textures.find(path);
	if (it != m_textures.end())
	{
		return it->second;
	}

	int newHandle = LoadGraph(path.c_str());
	if (newHandle == -1)
	{
		std::cerr << "‰æ‘œƒtƒ@ƒCƒ‹‚ð“Ç‚Ýž‚ß‚Ü‚¹‚ñ‚Å‚µ‚½\n";
		return -1;
	}

	m_textures[path] = newHandle;
	return newHandle;
}

void EventTexture::DeleteTexture(const std::string& path)
{
	auto it = m_textures.find(path);
	if (it != m_textures.end())
	{
		DeleteGraph(it->second);
		m_textures.erase(it);
	}
}

void EventTexture::Clear()
{
	for (auto& pair : m_textures)
	{
		DeleteGraph(pair.second);
	}
	m_textures.clear();
}