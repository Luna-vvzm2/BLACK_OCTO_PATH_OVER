#include "NewsViewer.h"
#include "Input.h"
#include "SpriteComponent.h"

#include <DxLib.h>
#include <algorithm>

namespace
{
    constexpr float FADE_IN_TIME = 0.2f;
    constexpr float MIN_SHOW_TIME = 0.5f;

    int LoadNewsImage(const std::string& path)
    {
        if (path.empty() || FileRead_size(path.c_str()) < 0) return -1;
        return SpriteComponent::LoadCachedTexture(path);
    }

    void DrawText(int x, int y, const std::string& text, unsigned int color, int font)
    {
        if (font >= 0) DrawStringToHandle(x, y, text.c_str(), color, font);
        else DrawString(x, y, text.c_str(), color);
    }

    int DrawWrappedText(int x, int y, int width, int bottom, int lineHeight,
        const std::string& text, unsigned int color, int font)
    {
        std::string line;
        for (size_t offset = 0; offset < text.size();)
        {
            if (text[offset] == '\r') { ++offset; continue; }
            if (text[offset] == '\n')
            {
                if (y + lineHeight > bottom) return y;
                DrawText(x, y, line, color, font);
                y += lineHeight;
                line.clear();
                ++offset;
                continue;
            }
            const size_t length = IsDBCSLeadByteEx(CP_ACP,
                static_cast<BYTE>(text[offset])) && offset + 1 < text.size() ? 2 : 1;
            const std::string character = text.substr(offset, length);
            const std::string candidate = line + character;
            const int measured = font >= 0
                ? GetDrawStringWidthToHandle(candidate.c_str(), static_cast<int>(candidate.size()), font)
                : GetDrawStringWidth(candidate.c_str(), static_cast<int>(candidate.size()));
            if (!line.empty() && measured > width)
            {
                if (y + lineHeight > bottom) return y;
                DrawText(x, y, line, color, font);
                y += lineHeight;
                line.clear();
            }
            line += character;
            offset += length;
        }
        if (!line.empty() && y + lineHeight <= bottom)
        {
            DrawText(x, y, line, color, font);
            y += lineHeight;
        }
        return y;
    }

    void DrawFittedImage(int handle, int x, int y, int width, int height)
    {
        int imageWidth = 0, imageHeight = 0;
        if (handle < 0 || GetGraphSize(handle, &imageWidth, &imageHeight) < 0 ||
            imageWidth <= 0 || imageHeight <= 0) return;
        const float scale = (std::min)(static_cast<float>(width) / imageWidth,
            static_cast<float>(height) / imageHeight);
        const float drawWidth = imageWidth * scale;
        const float drawHeight = imageHeight * scale;
        const float left = x + (width - drawWidth) * 0.5f;
        const float top = y + (height - drawHeight) * 0.5f;
        DrawExtendGraphF(left, top, left + drawWidth, top + drawHeight, handle, TRUE);
    }
}

NewsViewer::NewsViewer()
    : m_open(false), m_timer(0.0f), m_imageHandle(-1)
{
}

NewsViewer::~NewsViewer()
{
    for (int font : { m_headlineFont, m_bodyFont, m_captionFont })
        if (font >= 0) DeleteFontToHandle(font);
}

void NewsViewer::Open(const NewsData& news)
{
    m_title = news.title;
    m_date = news.date;
    m_body = news.body;
    m_imageHandle = LoadNewsImage(news.imagePath);
    m_portraitHandle = LoadNewsImage(news.portraitPath);
    if (m_headlineFont < 0) m_headlineFont = CreateFontToHandle("Yu Mincho", 34, 3, DX_FONTTYPE_ANTIALIASING);
    if (m_bodyFont < 0) m_bodyFont = CreateFontToHandle("Yu Mincho", 24, 1, DX_FONTTYPE_ANTIALIASING);
    if (m_captionFont < 0) m_captionFont = CreateFontToHandle("Yu Gothic", 19, 1, DX_FONTTYPE_ANTIALIASING);
    m_open = true;
    m_timer = 0.0f;
}

void NewsViewer::Close()
{
    m_open = false;
}

void NewsViewer::Update(float deltaTime, const Input& input)
{
    if (!m_open) return;
    m_timer += deltaTime;
    if (m_timer >= MIN_SHOW_TIME && input.IsMenuTrigger(Action::ENTER))
        Close();
}

void NewsViewer::Draw() const
{
    if (!m_open) return;
    int screenWidth = 1280, screenHeight = 720;
    GetDrawScreenSize(&screenWidth, &screenHeight);
    const float fade = (std::min)(m_timer / FADE_IN_TIME, 1.0f);
    int previousBlend = 0, previousAlpha = 0;
    GetDrawBlendMode(&previousBlend, &previousAlpha);
    const int previousDrawMode = GetDrawMode();
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(210 * fade));
    DrawBox(0, 0, screenWidth, screenHeight, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * fade));
    SetDrawMode(DX_DRAWMODE_BILINEAR);

    const bool hasPortrait = m_portraitHandle >= 0;
    const int paperWidth = static_cast<int>(screenWidth * 0.84f);
    const int paperHeight = static_cast<int>(screenHeight * 0.84f);
    const int paperLeft = (screenWidth - paperWidth) / 2;
    const int paperTop = static_cast<int>(screenHeight * 0.06f);

    if (m_imageHandle >= 0)
        DrawFittedImage(m_imageHandle, paperLeft, paperTop, paperWidth, paperHeight);
    else
    {
        const unsigned int ink = GetColor(43, 38, 31);
        const int margin = 32;
        const int textWidth = hasPortrait ? static_cast<int>(paperWidth * 0.55f) - margin
            : paperWidth - margin * 2;
        DrawBox(paperLeft + 7, paperTop + 7, paperLeft + paperWidth + 7,
            paperTop + paperHeight + 7, GetColor(12, 10, 7), TRUE);
        DrawBox(paperLeft, paperTop, paperLeft + paperWidth, paperTop + paperHeight,
            GetColor(242, 232, 208), TRUE);
        DrawBox(paperLeft, paperTop, paperLeft + paperWidth, paperTop + paperHeight, ink, FALSE);
        DrawText(paperLeft + margin, paperTop + 22, "êVï∑", ink, m_bodyFont);
        DrawText(paperLeft + margin, paperTop + 58, m_date, ink, m_captionFont);
        DrawLine(paperLeft + margin, paperTop + 94, paperLeft + paperWidth - margin,
            paperTop + 94, ink, 2);
        const int headlineEnd = DrawWrappedText(paperLeft + margin, paperTop + 114,
            textWidth, paperTop + paperHeight - 30, 46, m_title, ink, m_headlineFont);
        DrawLine(paperLeft + margin, headlineEnd + 16, paperLeft + paperWidth - margin,
            headlineEnd + 16, ink);
        DrawWrappedText(paperLeft + margin, headlineEnd + 42, textWidth,
            paperTop + paperHeight - 30, 36, m_body, ink, m_bodyFont);
    }

    if (hasPortrait)
    {
        const int portraitWidth = static_cast<int>(paperWidth * 0.40f);
        const int portraitLeft = paperLeft + paperWidth - portraitWidth - 24;
        const int portraitTop = paperTop + 112;
        // Draw directly over the paper; keep the PNG's alpha channel intact.
        DrawFittedImage(m_portraitHandle, portraitLeft, portraitTop,
            portraitWidth, paperHeight - 136);
    }

    if (m_timer >= MIN_SHOW_TIME)
    {
        const std::string hint = "Enter / BÅFï¬Ç∂ÇÈ";
        const int hintWidth = m_captionFont >= 0
            ? GetDrawStringWidthToHandle(hint.c_str(), static_cast<int>(hint.size()), m_captionFont)
            : GetDrawStringWidth(hint.c_str(), static_cast<int>(hint.size()));
        DrawText((screenWidth - hintWidth) / 2, screenHeight - 43,
            hint, GetColor(255, 255, 255), m_captionFont);
    }
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}