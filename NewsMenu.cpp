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
        const char* body;
        const char* image;   // 右上に表示する画像のパス (なければ省略 = nullptr)
    };

    const NewsInfo NEWS_LIST[NewsMenu::NewsCount] =
    {
        { "金城失踪から早10日",       "2000年5月23日",
          "こんにちは。 てすとです。",
          "assets/images/news/news1.png" },
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
    for (int i = 0; i < NewsCount; i++)
    {
        m_imageHandle[i] = -1;
    }
}

void NewsMenu::Initialize()
{
    // 画像の読み込み
    for (int i = 0; i < NewsCount; i++)
    {
        if (m_imageHandle[i] >= 0) continue;   // 二重読み込み防止

        if (NEWS_LIST[i].image != nullptr)
        {
            m_imageHandle[i] = LoadGraph(NEWS_LIST[i].image);
        }
    }
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
            DrawString(listX + 16, y + 34, "？？？", color);
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

        //----------------------------------
        // 右上: 画像 (縦横比を保って枠内に収める)
        //----------------------------------
        const int imgAreaW = 240;
        const int imgAreaH = 160;
        const int imgAreaX = boxX + boxW - 30 - imgAreaW;
        const int imgAreaY = boxY + 30;

        const int handle = m_imageHandle[sel];
        if (handle >= 0)
        {
            int w = 0;
            int h = 0;
            GetGraphSize(handle, &w, &h);

            if (w > 0 && h > 0)
            {
                const float sx = static_cast<float>(imgAreaW) / w;
                const float sy = static_cast<float>(imgAreaH) / h;
                const float scale = (sx < sy) ? sx : sy;

                const int dw = static_cast<int>(w * scale);
                const int dh = static_cast<int>(h * scale);
                const int x = imgAreaX + imgAreaW - dw;   // 右揃え

                DrawExtendGraph(x, imgAreaY, x + dw, imgAreaY + dh, handle, TRUE);
            }
        }

        //----------------------------------
        // 本文 (画像の下から)
        //----------------------------------
        if (NEWS_LIST[sel].body != nullptr)
        {
            DrawString(boxX + 30, boxY + 210, NEWS_LIST[sel].body, white);
        }
    }
    else
    {
        DrawString(boxX + 30, boxY + 30, "？？？", gray);
    }
}

void NewsMenu::ClampCursor()
{
    m_owner->GetCursor().ClampIndex(NewsCount - 1);
}
