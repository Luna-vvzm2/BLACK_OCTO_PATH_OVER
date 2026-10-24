#include "Menu.h"

#include "PlayScene.h"
#include "Game.h"

Menu::Menu(PlayScene* owner)
	:
	m_owner(owner),
	m_skillMenu(this),
	m_equipmentMenu(this),
	m_keyConfigMenu(this)
{
}

Game* Menu::GetGame() const
{
	return m_owner->GetGame();
}

void Menu::SaveSettings()
{
	if (m_owner)
	{
		m_owner->SaveCurrentSettings();
	}
}


void Menu::Initialize()
{
	m_skillMenu.Initialize();
	m_equipmentMenu.Initialize();
	m_keyConfigMenu.Initialize();
}

void Menu::Update(float deltaTime)
{
	const Input& input = m_owner->GetGame()->GetInput();

	if (m_cursorArea == MenuCursorArea::Tab)
	{
		if (input.IsMenuTrigger(Action::RIGHT))
		{
			m_tabCursor = (m_tabCursor + 1) % 3;
		}

		if (input.IsTrigger(Action::LEFT))
		{
			m_tabCursor = (m_tabCursor + 2) % 3;
		}

		if (input.IsTrigger(Action::DOWN) ||
			input.IsTrigger(Action::ENTER))
		{
			switch (m_tabCursor)
			{
			case 0:
				m_currentTab = Tab::Skill;
				m_cursorArea = MenuCursorArea::SkillList;
				break;

			case 1:
				m_currentTab = Tab::Equipment;
				m_cursorArea = MenuCursorArea::EquipmentList;
				break;

			case 2:
				m_currentTab = Tab::KeyConfig;
				m_cursorArea = MenuCursorArea::KeyConfigList;
				break;
			}

			return;
		}

		return;
	}

	if (m_currentTab == Tab::Skill)
	{
		m_skillMenu.Update(deltaTime);
	}
	else if (m_currentTab == Tab::Equipment)
	{
		m_equipmentMenu.Update(deltaTime);
	}
	else
	{
		m_keyConfigMenu.Update(deltaTime);
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
	{
		m_skillMenu.Draw();
	}
	else if (m_currentTab == Tab::Equipment)
	{
		m_equipmentMenu.Draw();
	}
	else
	{
		m_keyConfigMenu.Draw();
	}
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

	const int totalWidth = tabW * 3 + tabGap * 2;

	const int startX =
		(m_owner->GetGame()->GetWidth() - totalWidth) / 2;

	const int skillX = startX;
	const int equipmentX = startX + tabW + tabGap;
	const int keyConfigX = equipmentX + tabW + tabGap;

	auto drawTab = [&](int x, const char* label, int textOffset)
		{
			DrawBox(
				x,
				tabY,
				x + tabW,
				tabY + tabH,
				GetColor(80, 80, 80),
				TRUE
			);

			DrawString(
				x + textOffset,
				tabY + 15,
				label,
				GetColor(255, 255, 255)
			);
		};

	drawTab(skillX, "スキル", 60);
	drawTab(equipmentX, "装備", 70);
	drawTab(keyConfigX, "操作設定", 45);

	if (m_cursorArea == MenuCursorArea::Tab)
	{
		int x = startX + m_tabCursor * (tabW + tabGap);

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

bool Menu::IsCapturingKey() const
{
	return m_isOpen &&
		m_currentTab == Tab::KeyConfig &&
		m_keyConfigMenu.IsWaitingForKey();
}