#include "SkillMenu.h"

#include "Menu.h"
#include "Game.h"

#include <DxLib.h>

EquipmentMenu::EquipmentMenu(Menu* owner)
	: MenuPage(owner)
{
}

void EquipmentMenu::Initialize()
{
}

void EquipmentMenu::Update(float deltaTime)
{
	auto& cursor = m_owner->GetCursor();

	const Input& input =
		m_owner->GetGame()->GetInput();

	//----------------------------------
	// Enter
	//----------------------------------
	if (input.IsTrigger(Action::ENTER))
	{
		if (!m_inventoryMode)
		{
			// 所持装備へ入る
			m_inventoryMode = true;

			m_prevCategory = cursor.GetCategory();
			m_prevIndex = cursor.GetIndex();

			cursor.SetCategory(3);
			cursor.SetIndex(0);
		}
		else
		{
			// 元の位置へ戻る
			m_inventoryMode = false;

			cursor.SetCategory(m_prevCategory);
			cursor.SetIndex(m_prevIndex);
		}
	}

	//----------------------------------
	// 所持装備モード
	//----------------------------------
	if (m_inventoryMode)
	{
		if (cursor.IsRepeat(Action::LEFT,input,deltaTime))
		{
			if (cursor.GetIndex() % 5 != 0)
				cursor.SetIndex(cursor.GetIndex() - 1);
		}

		if (cursor.IsRepeat(Action::RIGHT, input, deltaTime))
		{
			if (cursor.GetIndex() % 5 != 4 &&
				cursor.GetIndex() < 9)
			{
				cursor.SetIndex(cursor.GetIndex() + 1);
			}
		}

		if (cursor.IsRepeat(Action::UP, input, deltaTime))
		{
			if (cursor.GetIndex() >= 5)
			{
				cursor.SetIndex(cursor.GetIndex() - 5);
			}
		}

		if (cursor.IsRepeat(Action::DOWN, input, deltaTime))
		{
			if (cursor.GetIndex() < 5)
			{
				cursor.SetIndex(cursor.GetIndex() + 5);
			}
		}
	}
	//----------------------------------
	// 通常装備カーソル
	//----------------------------------
	else{
	//----------------------
	// 左
	//----------------------
	if (cursor.IsRepeat(Action::LEFT, input, deltaTime))
	{
		switch (cursor.GetCategory())
		{
		case 1:     // 護符 → 忍法
			cursor.SetCategory(0);
			if (cursor.GetIndex() > 3)
			{
				cursor.SetIndex(3);
			}
			break;

		case 2:     // 刀 → 忍法(一番下)
			cursor.SetCategory(0);
			cursor.SetIndex(3);
			break;
		
		case 3:     // 所持装備
			if (cursor.GetIndex() % 5 != 0)
				cursor.SetIndex(cursor.GetIndex() - 1);
			break;
		}
	}
	//----------------------
	// 右
	//----------------------
	if (cursor.IsRepeat(Action::RIGHT, input, deltaTime))
	{
		switch (cursor.GetCategory())
		{
		case 0:
			if (cursor.GetIndex() == 3)
			{
				// 一番下なら刀
				cursor.SetCategory(2);
				cursor.SetIndex(0);
			}
			else
			{
				// 護符へ
				cursor.SetCategory(1);
				if (cursor.GetIndex() > 1)
				{
					cursor.SetIndex(1);
				}
			}
			break;

		case 3:
			if (cursor.GetIndex() % 5 != 4)
				cursor.SetIndex(cursor.GetIndex() + 1);
			break;
		}
	}

	//----------------------
	// 上
	//----------------------
	if (cursor.IsRepeat(Action::UP, input, deltaTime))
	{
		switch (cursor.GetCategory())
		{
		case 0:
			if (cursor.GetIndex() == 0)
			{
				m_owner->SetCursorArea(Menu::MenuCursorArea::Tab);
			}
			else
			{
				cursor.SetIndex(cursor.GetIndex() - 1);
			}
			break;

		case 1:
			if (cursor.GetIndex() == 0)
			{
				m_owner->SetCursorArea(Menu::MenuCursorArea::Tab);
			}
			else
			{
				cursor.SetIndex(cursor.GetIndex() - 1);
			}
			break;

		case 2:
			cursor.SetCategory(1);
			cursor.SetIndex(1);
			break;

		case 3:
			if (cursor.GetIndex() >= 5)
			{
				cursor.SetIndex(cursor.GetIndex() - 5);
			}
			break;
		}
	}
		//----------------------
		// 下
		//----------------------
	if (cursor.IsRepeat(Action::DOWN, input, deltaTime))
		{
			switch (cursor.GetCategory())
			{
			case 0:
				if (cursor.GetIndex() < 3)
				{
					cursor.SetIndex(cursor.GetIndex() + 1);
				}
				break;

			case 1:
				if (cursor.GetIndex() == 0)
				{
					cursor.SetIndex(1);
				}
				else
				{
					cursor.SetCategory(2);
					cursor.SetIndex(0);
				}
				break;

			case 2:

				break;

			case 3:
				if (cursor.GetIndex() < 5)
					cursor.SetIndex(cursor.GetIndex() + 5);
				break;
			}
		}

		ClampCursor();
	}	
}

void EquipmentMenu::Draw()
{
	DrawNinjutsuSlots();
	DrawGofuSlots();
	DrawKatanaSlot();
	DrawEquipmentInventory();
	DrawEquipmentDescription();

	if (m_owner->GetCursorArea() != Menu::MenuCursorArea::EquipmentList)
	{
		return;
	}

	//----------------------------------
	// カーソル
	//----------------------------------
		auto& cursor = m_owner->GetCursor();

		int cursorX = 0;
		int cursorY = 0;
		int cursorW = 70;
		int cursorH = 70;

		switch (cursor.GetCategory())
		{
		case 0:     // 忍法
			cursorX = 50;
			cursorY = 120 + cursor.GetIndex() * 90;
			cursorW = 70;
			cursorH = 70;
			break;

		case 1:     // 護符
			cursorX = 470;
			cursorY = 120 + cursor.GetIndex() * 100;
			cursorW = 70;
			cursorH = 70;
			break;

		case 2:     // 刀
			cursorX = 470;
			cursorY = 390;
			cursorW = 70;
			cursorH = 70;
			break;

		case 3:     // 所持装備
		{
			const int size = 60;
			const int gap = 15;

			cursorX = 50 + (cursor.GetIndex() % 5) * (size + gap);
			cursorY = 560 + (cursor.GetIndex() / 5) * (size + gap);

			// ←ここだけ小さくする
			cursorW = 60;
			cursorH = 60;
			break;
		}
		}

		DrawBox(
			cursorX - 4,
			cursorY - 4,
			cursorX + cursorW + 4,
			cursorY + cursorH + 4,
			GetColor(255, 0, 0),
			FALSE
		);
	}

void EquipmentMenu::DrawNinjutsuSlots()
{
    const int x = 50;
    const int y = 120;
    const int size = 70;
    const int gap = 20;

    DrawString(
        x,
        y - 40,
        "忍法",
        GetColor(255, 255, 255)
    );

    for (int i = 0; i < 4; i++)
    {
        DrawBox(
            x,
            y + i * (size + gap),
            x + size,
            y + i * (size + gap) + size,
            GetColor(255, 255, 255),
            FALSE
        );
    }
}

void EquipmentMenu::DrawGofuSlots()
{
	const int x = 350;
	const int y = 120;
	const int size = 70;
	const int gap = 30;

	DrawString(
		x,
		y - 40,
		"護符",
		GetColor(255, 255, 255)
	);

	// 自動発動
	DrawString(
		x,
		y + 20,
		"自動発動",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);

	// 連撃発動
	DrawString(
		x,
		y + size + gap + 20,
		"連撃発動",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y + size + gap,
		x + 120 + size,
		y + size * 2 + gap,
		GetColor(255, 255, 255),
		FALSE
	);
}

void EquipmentMenu::DrawKatanaSlot()
{
	const int x = 350;
	const int y = 390;
	const int size = 70;

	DrawString(
		x,
		y - 40,
		"刀",
		GetColor(255, 255, 255)
	);

	// 刀名
	DrawString(
		x,
		y + 20,
		"朧月",
		GetColor(255, 255, 255)
	);

	// 装備枠
	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);
}

void EquipmentMenu::DrawEquipmentInventory()
{
	const int startX = 50;
	const int startY = 560;

	const int size = 60;
	const int gap = 15;

	DrawString(
		startX,
		startY - 40,
		"所持装備",
		GetColor(255, 255, 255)
	);

	for (int i = 0; i < 10; i++)
	{
		int x = startX + (i % 5) * (size + gap);
		int y = startY + (i / 5) * (size + gap);

		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}
}

void EquipmentMenu::DrawEquipmentDescription()
{
	DrawBox(
		650,
		120,
		1220,
		680,
		GetColor(255, 255, 255),
		FALSE
	);

	DrawString(
		680,
		150,
		"説明",
		GetColor(255, 255, 255)
	);
}

void EquipmentMenu::ClampCursor()
{
	auto& cursor = m_owner->GetCursor();

	cursor.ClampCategory(3);

	switch (cursor.GetCategory())
	{
	case 0:     // 忍法
		cursor.ClampIndex(3);
		break;

	case 1:     // 護符
		cursor.ClampIndex(1);
		break;

	case 2:     // 刀
		cursor.ClampIndex(0);
		break;

	case 3:     // 所持装備
		cursor.ClampIndex(9);
		break;
	}
}

Vector2d EquipmentMenu::GetCursorPosition() const
{
	auto& cursor = m_owner->GetCursor();

	switch (cursor.GetCategory())
	{
	case 0:
		return {
			50.0f,
			120.0f + cursor.GetIndex() * 90.0f
		};

	case 1:
		return {
			470.0f,
			120.0f + cursor.GetIndex() * 100.0f
		};

	case 2:
		return {
			470.0f,
			390.0f
		};

	case 3:
	{
		int index = cursor.GetIndex();

		return {
			50.0f + (index % 5) * 75.0f,
			560.0f + (index / 5) * 75.0f
		};
	}
	}

	return {};
}