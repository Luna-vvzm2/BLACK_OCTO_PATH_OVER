#include "SubMenu.h"
#include "SubMenuMapPage.h"
#include "SubMenuStatusPage.h"
#include "SubMenuNewsPage.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "DxLib.h" 
SubMenu::SubMenu(Scene* scene)
    : UIActor(scene)
    , m_isOpen(false)
    , m_currentTab(Tab::Map)
    , m_player(nullptr)
    , m_newsList(GetDefaultNewsList())
    , m_mapPage(std::make_unique<SubMenuMapPage>(this))
    , m_statusPage(std::make_unique<SubMenuStatusPage>(this))
    , m_newsPage(std::make_unique<SubMenuNewsPage>(this))
    , m_tabSwitchTimer(0.0f)
{
    SetName("SubMenu");
}

SubMenu::~SubMenu() = default;

bool SubMenu::Init()
{
    if (!UIActor::Init()) return false;

    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (playScene)
    {
        m_player = playScene->GetPlayer();
    }

    if (m_mapPage)    m_mapPage->Initialize();
    if (m_statusPage) m_statusPage->Initialize();
    if (m_newsPage)   m_newsPage->Initialize();

    return true;
}

void SubMenu::Open()
{
    m_isOpen = true;
    m_currentTab = Tab::Map; // 開いた直後は「マップ」タブを初期表示
    m_tabSwitchTimer = 0.0f;

    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (playScene)
    {
        m_player = playScene->GetPlayer();
    }
}

void SubMenu::Close()
{
    m_isOpen = false;
}

void SubMenu::Toggle()
{
    if (m_isOpen) Close();
    else          Open();
}

void SubMenu::UnlockNews(int newsId)
{
    for (auto& news : m_newsList)
    {
        if (news.id == newsId)
        {
            news.unlocked = true;
            break;
        }
    }
}

void SubMenu::Update(float deltaTime)
{
    if (!m_isOpen) return;

    UIActor::Update(deltaTime);

    const Input& input = m_scene->GetGame()->GetInput();

    // 閉じる処理 (Mキー または Escキー)
    if (input.IsTrigger(Action::MENU) || input.GetKey().IsTrigger(Key::ESCAPE))
    {
        Close();
        return;
    }

    // タブ切り替え (PageUp / PageDown)
    UpdateTabInput();

    // 選択中タブの更新
    switch (m_currentTab)
    {
    case Tab::Map:    if (m_mapPage)    m_mapPage->Update(deltaTime);    break;
    case Tab::Status: if (m_statusPage) m_statusPage->Update(deltaTime); break;
    case Tab::News:   if (m_newsPage)   m_newsPage->Update(deltaTime);   break;
    default: break;
    }
}

void SubMenu::UpdateTabInput()
{
    const Input& input = m_scene->GetGame()->GetInput();

    int tabIndex = static_cast<int>(m_currentTab);
    const int maxTab = static_cast<int>(Tab::Max);

    if (input.GetKey().IsTrigger(Key::UP))
    {
        tabIndex = (tabIndex - 1 + maxTab) % maxTab;
        m_currentTab = static_cast<Tab>(tabIndex);
    }
    else if (input.GetKey().IsTrigger(Key::DOWN))
    {
        tabIndex = (tabIndex + 1) % maxTab;
        m_currentTab = static_cast<Tab>(tabIndex);
    }
}

void SubMenu::Draw()
{
    if (!m_isOpen) return;

    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    if (!renderer) return;

    // 半透明背景
    renderer->DrawFullScreenFill(Color(0, 0, 0), 200);

    // メインウィンドウ背景
    DrawBox(80, 40, 1200, 680, GetColor(30, 30, 35), TRUE);
    DrawBox(80, 40, 1200, 680, GetColor(150, 150, 160), FALSE);

    DrawHeader();

    // 各タブ画面の描画
    switch (m_currentTab)
    {
    case Tab::Map:    if (m_mapPage)    m_mapPage->Draw();    break;
    case Tab::Status: if (m_statusPage) m_statusPage->Draw(); break;
    case Tab::News:   if (m_newsPage)   m_newsPage->Draw();   break;
    default: break;
    }

    // 操作ガイド表示
    const std::string& font = m_scene->GetGame()->GatDebugFont();
    renderer->DrawTextL(Vector2d(100.0f, 645.0f),
        "[PageUp/PageDown]: タブ切替  [M / Esc]: 閉じる",
        Color(200, 200, 200), font, 18, false);

    UIActor::Draw();
}

void SubMenu::DrawHeader()
{
    Renderer* renderer = m_scene->GetGame()->GetRenderer();
    const std::string& font = m_scene->GetGame()->GatDebugFont();

    const char* tabNames[] = { "マップ", "ステータス", "新聞" };
    const float tabWidth = 200.0f;
    const float startX = 340.0f;
    const float startY = 60.0f;

    for (int i = 0; i < static_cast<int>(Tab::Max); ++i)
    {
        float x1 = startX + i * (tabWidth + 20.0f);
        float x2 = x1 + tabWidth;
        float y1 = startY;
        float y2 = y1 + 40.0f;

        bool isSelected = (i == static_cast<int>(m_currentTab));

        int bgColor = isSelected ? GetColor(100, 100, 120) : GetColor(50, 50, 60);
        int lineColor = isSelected ? GetColor(255, 215, 0) : GetColor(100, 100, 100);

        DrawBox(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), bgColor, TRUE);
        DrawBox(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), lineColor, FALSE);

        Color textColor = isSelected ? Color(255, 255, 255) : Color(150, 150, 150);
        renderer->DrawTextC(Vector2d(x1 + tabWidth * 0.5f, y1 + 8.0f), tabNames[i], textColor, font, 20, false);
    }

    DrawLine(100, 110, 1180, 110, GetColor(150, 150, 160));
}