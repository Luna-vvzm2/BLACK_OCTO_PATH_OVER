#pragma once
#include <string>
#include <map>

class EventTexture
{
public:
	EventTexture();
	~EventTexture();

	int LoadTexture(const std::string& path);
	void DeleteTexture(const std::string& path);
	void Clear(); //デストラクタで呼ぶ保険の解放処理

private:
	std::map<std::string, int> m_textures;
};

