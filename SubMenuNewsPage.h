#pragma once
#include <string>
#include <unordered_map>
#include "NewsData.h"

class SubMenu;

class SubMenuNewsPage
{
public:
    explicit SubMenuNewsPage(SubMenu* owner);
    ~SubMenuNewsPage() = default;

    void Initialize();
    void Update(float deltaTime);
    void Draw();

private:
    int GetGraphHandle(const std::string& path);

private:
    SubMenu* m_owner = nullptr;
    int m_selectedIndex = 0; // 現在選択中の新聞インデックス
    std::unordered_map<std::string, int> m_imageGraphCache;
};