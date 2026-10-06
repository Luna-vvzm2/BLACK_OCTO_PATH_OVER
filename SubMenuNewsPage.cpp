#include "SubMenuNewsPage.h"
#include "SubMenu.h"
#include "Scene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "DxLib.h"
#include <string>
#include <unordered_map>

namespace
{
    void DrawPanel(float x, float y, float w, float h, int fillColor, int lineColor)
    {
        const int x1 = static_cast<int>(x);
        const int y1 = static_cast<int>(y);
        const int x2 = static_cast<int>(x + w);
        const int y2 = static_cast<int>(y + h);
        DrawBox(x1, y1, x2, y2, fillColor, TRUE);
        DrawBox(x1, y1, x2, y2, lineColor, FALSE);
    }
}

SubMenuNewsPage::SubMenuNewsPage(SubMenu* owner)
    : m_owner(owner)
    , m_selectedIndex(0)
{
}

void SubMenuNewsPage::Initialize()
{
    m_selectedIndex = 0;
}

int SubMenuNewsPage::GetGraphHandle(const std::string& path)
{
    if (path.empty()) return -1;

    auto it = m_imageGraphCache.find(path);
    if (it != m_imageGraphCache.end())
    {
        return it->second;
    }

    int handle = LoadGraph(path.c_str());
    m_imageGraphCache[path] = handle;
    return handle;
}

void SubMenuNewsPage::Update(float deltaTime)
{
    (void)deltaTime;
    if (!m_owner) return;

    const Input& input = m_owner->GetScene()->GetGame()->GetInput();
    const auto& newsList = m_owner->GetNewsList();
    const int total = static_cast<int>(newsList.size());

    if (total == 0) return;

    // 上下キーで選択変更 (W/S や 上下矢印)
    if (input.GetKey().IsTrigger(Key::UP) || input.GetKey().IsTrigger(Key::W))
    {
        m_selectedIndex = (m_selectedIndex - 1 + total) % total;
    }
    else if (input.GetKey().IsTrigger(Key::DOWN) || input.GetKey().IsTrigger(Key::S))
    {
        m_selectedIndex = (m_selectedIndex + 1) % total;
    }
}

void SubMenuNewsPage::Draw()
{
    if (!m_owner) return;

    Renderer* renderer = m_owner->GetScene()->GetGame()->GetRenderer();
    if (!renderer) return;
    const std::string& font = m_owner->GetScene()->GetGame()->GatDebugFont();
    const auto& newsList = m_owner->GetNewsList();

    // 左側:新聞一覧
    const float leftX = 100.0f;
    const float startY = 140.0f;
    const float itemWidth = 320.0f;
    const float itemHeight = 70.0f;
    const float gapY = 12.0f;

    for (size_t i = 0; i < newsList.size(); ++i)
    {
        const float y1 = startY + static_cast<float>(i) * (itemHeight + gapY);
        const bool isSelected = (static_cast<int>(i) == m_selectedIndex);

        const int bgColor = isSelected ? GetColor(80, 80, 110) : GetColor(45, 45, 55);
        const int lineColor = isSelected ? GetColor(255, 215, 0) : GetColor(90, 90, 100);
        DrawPanel(leftX, y1, itemWidth, itemHeight, bgColor, lineColor);

        const auto& news = newsList[i];
        if (news.unlocked)
        {
            Color titleColor = isSelected ? Color(255, 255, 255) : Color(200, 200, 200);
            renderer->DrawTextL(Vector2d(leftX + 15.0f, y1 + 12.0f), news.title, titleColor, font, 18, false);
            renderer->DrawTextL(Vector2d(leftX + 15.0f, y1 + 40.0f), news.date, Color(150, 150, 160), font, 14, false);
        }
        else
        {
            renderer->DrawTextL(Vector2d(leftX + 15.0f, y1 + 22.0f), "???  【 未入手 】", Color(120, 120, 120), font, 18, false);
        }
    }

    // 右側:選択中の新聞画像
    const float rightX = 450.0f;
    const float rightY = 140.0f;
    const float rightW = 710.0f;
    const float rightH = 480.0f;

    DrawPanel(rightX, rightY, rightW, rightH, GetColor(20, 20, 25), GetColor(100, 100, 120));

    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(newsList.size()))
    {
        const auto& selectedNews = newsList[m_selectedIndex];

        if (selectedNews.unlocked)
        {
            const int handle = GetGraphHandle(selectedNews.imagePath);
            if (handle >= 0)
            {
                DrawExtendGraph(
                    static_cast<int>(rightX + 10.0f),
                    static_cast<int>(rightY + 10.0f),
                    static_cast<int>(rightX + rightW - 10.0f),
                    static_cast<int>(rightY + rightH - 10.0f),
                    handle,
                    TRUE);
            }
            else
            {
                renderer->DrawTextC(
                    Vector2d(rightX + rightW * 0.5f, rightY + rightH * 0.5f),
                    std::string("新聞画像が見つかりません:\n") + selectedNews.imagePath,
                    Color(255, 100, 100), font, 18, false);
            }
        }
        else
        {
            renderer->DrawTextC(
                Vector2d(rightX + rightW * 0.5f, rightY + rightH * 0.5f),
                "この新聞はまだ入手していません",
                Color(150, 150, 150), font, 22, false);
        }
    }
}