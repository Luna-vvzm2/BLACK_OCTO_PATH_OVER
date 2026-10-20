#include "Menu.h"

#include "PlayScene.h"
#include "Game.h"
#include "Input.h"

#include <DxLib.h>
#include <cstring>

Menu::Menu(PlayScene* owner)
	: m_owner(owner)
	, m_mapMenu(this)
	, m_statusMenu(this)
	, m_newsMenu(this)
{
}

Game* Menu::GetGame() const
{
	return m_owner->GetGame();
}

PlayerEntity* Menu::GetPlayer() const
{
	return m_owner->GetPlayer();
}

void Menu::Initialize()
{
	m_mapMenu.Initialize();
	m_statusMenu.Initialize();
	m_newsMenu.Initialize();
}

void Menu::ChangeTab(int delta)
{
	int next = static_cast<int>(m_currentTab) + delta;

	// 端では止まる
	if (next < 0 || next >= static_cast<int>(Tab::Count))
	{
		return;
	}

	m_currentTab = static_cast<Tab>(next);
	m_cursor.SetPosition(0, 0);
}

void Menu::Update(float deltaTime)
{
	const Input& input = GetGame()->GetInput();

	//--------------------
	// タブ切替 (PageUp/PageDown, LB/RB)
	//--------------------
	if (input.IsTrigger(Action::PAGE_UP))
	{
		ChangeTab(-1);
	}

	if (input.IsTrigger(Action::PAGE_DOWN))
	{
		ChangeTab(+1);
	}

	//--------------------
	// 各ページ更新
	//--------------------
	switch (m_currentTab)
	{
	case Tab::Map:
		m_mapMenu.Update(deltaTime);
		break;

	case Tab::Status:
		m_statusMenu.Update(deltaTime);
		break;

	case Tab::News:
		m_newsMenu.Update(deltaTime);
		break;

	default:
		break;
	}
}

void Menu::Draw()
{
	DrawBox(
		0,
		0,
		GetGame()->GetWidth(),
		GetGame()->GetHeight(),
		GetColor(20, 20, 20),
		TRUE
	);

	DrawTabs();

	switch (m_currentTab)
	{
	case Tab::Map:
		m_mapMenu.Draw();
		break;

	case Tab::Status:
		m_statusMenu.Draw();
		break;

	case Tab::News:
		m_newsMenu.Draw();
		break;

	default:
		break;
	}
}

//====================
// タブ
//====================

void Menu::DrawTabs() const
{
	static const char* labels[] = { "マップ", "ステータス", "新聞" };

	const int tabY = 20;
	const int tabW = 180;
	const int tabH = 50;
	const int tabGap = 10;
	const int tabCount = static_cast<int>(Tab::Count);

	const int totalWidth = tabW * tabCount + tabGap * (tabCount - 1);
	const int startX = (GetGame()->GetWidth() - totalWidth) / 2;

	for (int i = 0; i < tabCount; i++)
	{
		const int x = startX + i * (tabW + tabGap);
		const bool active = (i == static_cast<int>(m_currentTab));

		DrawBox(
			x,
			tabY,
			x + tabW,
			tabY + tabH,
			active ? GetColor(110, 110, 110) : GetColor(60, 60, 60),
			TRUE
		);

		const int textW = GetDrawStringWidth(
			labels[i],
			static_cast<int>(std::strlen(labels[i]))
		);

		DrawString(
			x + (tabW - textW) / 2,
			tabY + 17,
			labels[i],
			active ? GetColor(255, 255, 255) : GetColor(170, 170, 170)
		);

		if (active)
		{
			DrawBox(
				x - 4,
				tabY - 4,
				x + tabW + 4,
				tabY + tabH + 4,
				GetColor(255, 0, 0),
				FALSE
			);
		}
	}

	// 操作ヒント
	DrawString(startX - 150, tabY + 17, "< PgUp / LB", GetColor(130, 130, 130));
	DrawString(startX + totalWidth + 20, tabY + 17, "PgDn / RB >", GetColor(130, 130, 130));
}

bool Menu::IsOpen() const
{
	return m_isOpen;
}

void Menu::Toggle()
{
	m_isOpen = !m_isOpen;

	if (m_isOpen)
	{
		// 開いた直後はマップタブ
		m_currentTab = Tab::Map;
		m_cursor.SetPosition(0, 0);
	}
}

Menu::Tab Menu::GetCurrentTab() const
{
	return m_currentTab;
}
