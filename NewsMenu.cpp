#include "NewsMenu.h"

#include "Menu.h"
#include "Game.h"

#include <DxLib.h>

namespace
{
    struct NewsInfo
    {
        const char* title;
        const char* date;
    };

    const NewsInfo NEWS_LIST[NewsMenu::NewsCount] =
    {
        { "金城失踪から早10日",       "2000年5月23日" },
        { "黒い怪物にご用心",         "1987年10月8日" },
        { "話題のルーキー『金城清誠』", "1995年7月5日" },
        { "大江山近辺から国民を保護",   "1970年6月16日" },
        { "貢献者への勲章授与式",       "1970年7月1日" },
        { "噂の『影の魔術師』の行方",   "1901年2月11日" }
    };
}

NewsMenu::NewsMenu(Menu* owner)
    : MenuPage(owner)
{
}

void NewsMenu::Initialize()
{
#ifdef _DEBUG
    // デバッグ時のみ全て入手済みにして表示確認できるようにする
    for (int i = 0; i < NewsCount; i++)
    {
        m_owned[i] = true;
    }
#endif
}

void NewsMenu::SetOwned(int index, bool owned)
{
    if (index < 0 || index >= NewsCount)
    {
        return;
    }

    m_owned[index] = owned;
}

void NewsMenu::Update(float deltaTime)
{
    auto& cursor = m_owner->GetCursor();
    const Input& input = m_owner->GetGame()->GetInput();

    if (cursor.IsRepeat(Action::UP, input, deltaTime))
    {
        cursor.SetIndex(cursor.GetIndex() - 1);
    }

    if (cursor.IsRepeat(Action::DOWN, input, deltaTime))
    {
        cursor.SetIndex(cursor.GetIndex() + 1);
    }

    ClampCursor();
}

void NewsMenu::Draw()
{
    auto& cursor = m_owner->GetCursor();
    const unsigned int white = GetColor(255, 255, 255);
    const unsigned int gray = GetColor(90, 90, 90);

    //----------------------------------
    // 左: 一覧
    //----------------------------------
    const int listX = 60;
    const int listY = 120;
    const int itemW = 440;
    const int itemH = 64;
    const int itemGap = 16;

    for (int i = 0; i < NewsCount; i++)
    {
        const int y = listY + i * (itemH + itemGap);
        const unsigned int color = m_owned[i] ? white : gray;

        DrawBox(listX, y, listX + itemW, y + itemH, color, FALSE);
        DrawFormatString(listX + 16, y + 10, color, "No.%d", i + 1);

        if (m_owned[i])
        {
            DrawString(listX + 16, y + 34, NEWS_LIST[i].title, color);
        }
        else
        {
            DrawString(listX + 16, y + 34, "未入手", color);
        }
    }

    // カーソル
    {
        const int y = listY + cursor.GetIndex() * (itemH + itemGap);

        DrawBox(
            listX - 5,
            y - 5,
            listX + itemW + 5,
            y + itemH + 5,
            GetColor(255, 0, 0),
            FALSE
        );
    }

    //----------------------------------
    // 右: 本文
    //----------------------------------
    const int boxX = 540;
    const int boxY = 120;
    const int boxW = 680;
    const int boxH = 520;

    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, white, FALSE);

    const int sel = cursor.GetIndex();
    if (sel < 0 || sel >= NewsCount)
    {
        return;
    }

    if (m_owned[sel])
    {
        DrawString(boxX + 30, boxY + 30, NEWS_LIST[sel].title, white);
        DrawString(boxX + 30, boxY + 60, NEWS_LIST[sel].date, GetColor(180, 180, 180));
        DrawString(boxX + 30, boxY + 120, "（本文未定）", white);
    }
    else
    {
        DrawString(boxX + 30, boxY + 30, "未入手", gray);
    }
}

void NewsMenu::ClampCursor()
{
    m_owner->GetCursor().ClampIndex(NewsCount - 1);
}
