#include "Menu.h"

#include "PlayScene.h"
#include "Game.h"

Menu::Menu(PlayScene* owner)
	:
	m_owner(owner),
	m_skillMenu(this),
	m_equipmentMenu(this)
{
}

Game* Menu::GetGame() const
{
	return m_owner->GetGame();
}

void Menu::Initialize()
{
    m_skillMenu.Initialize();
    m_equipmentMenu.Initialize();
}

void Menu::Update(float deltaTime)
{
	const Input& input =
		m_owner->GetGame()->GetInput();

	//--------------------
	// タブ選択中
	//--------------------

	if (m_cursorArea == MenuCursorArea::Tab)
	{
		if (input.IsTrigger(Action::RIGHT))
			m_tabCursor = 0;

		if (input.IsTrigger(Action::LEFT))
			m_tabCursor = 1;

		if (input.IsTrigger(Action::DOWN) ||
			input.IsTrigger(Action::ENTER))
		{
			if (m_tabCursor == 0)
			{
				m_currentTab = Tab::Skill;
				m_cursorArea = MenuCursorArea::SkillList;
			}
			else 
			{
				m_currentTab = Tab::Equipment;//本来はEquipment
				m_cursorArea = MenuCursorArea::EquipmentList;
			}
			return;   
		}
	}

	//--------------------
	// 各メニュー更新
	//--------------------

	if (m_currentTab == Tab::Skill)
	{
		m_skillMenu.Update(deltaTime);
	}
	else
	{
		m_equipmentMenu.Update(deltaTime);
	}
}

void Menu::Draw()
{
	DrawBox(
		0,
		0,
		m_owner->GetGame()->GetWidth(),
		m_owner->GetGame()->GetHeight(),
		GetColor(20, 20, 20),
		TRUE
	);

	DrawTabs();

	if (m_currentTab == Tab::Skill)
		m_skillMenu.Draw();
	else
		m_equipmentMenu.Draw();
}

//====================
// タブ
//====================

void Menu::DrawTabs()
{
	const int tabY = 20;
	const int tabW = 180;
	const int tabH = 50;
	const int tabGap = 10;

	// タブ全体の幅
	const int totalWidth = tabW * 2 + tabGap;

	// 画面中央
	const int equipX = (m_owner->GetGame()->GetWidth() - totalWidth) / 2;
	const int skillX = equipX + tabW + tabGap;

	//====================
	// 装備
	//====================
	DrawBox(
		equipX,
		tabY,
		equipX + tabW,
		tabY + tabH,
		GetColor(80, 80, 80),
		TRUE
	);

	DrawString(
		equipX + 70,
		tabY + 15,
		"装備",
		GetColor(255, 255, 255)
	);

	//====================
	// スキル
	//====================
	DrawBox(
		skillX,
		tabY,
		skillX + tabW,
		tabY + tabH,
		GetColor(80, 80, 80),
		TRUE
	);

	DrawString(
		skillX + 60,
		tabY + 15,
		"スキル",
		GetColor(255, 255, 255)
	);

	//====================
	// タブカーソル
	//====================
	if (GetCursorArea() == MenuCursorArea::Tab)
	{
		int x = (m_tabCursor == 0) ? skillX : equipX;

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

bool Menu::IsOpen() const
{
	return m_isOpen;
}

void Menu::Toggle()
{
	m_isOpen = !m_isOpen;

	if (m_isOpen)
	{
		m_cursorArea = MenuCursorArea::Tab;
	}
}

Menu::MenuCursorArea Menu::GetCursorArea() const
{
	return m_cursorArea;
}

int Menu::GetTabCursor() const
{
	return m_tabCursor;
}

void Menu::SetCursorArea(MenuCursorArea area)
{
	m_cursorArea = area;
}

int Menu::GetLockedSkillIcon() const
{
	return m_lockedSkillIcon;
}