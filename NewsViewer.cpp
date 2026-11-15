#include "NewsViewer.h"
#include "Input.h"

#include <DxLib.h>
#include <algorithm>
#include <unordered_map>

namespace
{
    // 画面サイズ(PlayScene と同じ)
    constexpr int SCREEN_W = 1280;
    constexpr int SCREEN_H = 720;

    // 新聞画像を収める枠(画面中央)
    constexpr int IMAGE_AREA_W = 1080;
    constexpr int IMAGE_AREA_H = 600;

    constexpr float FADE_IN_TIME = 0.2f;        // 表示のフェードイン
    constexpr float MIN_SHOW_TIME = 0.5f;       // 開いてから、この時間は決定キーを受け付けない(誤入力防止)

    // 決定キー(キーボード: Enter / パッド: B)
    constexpr Action DECISION_ACTION = Action::ENTER;

    // 画像のハンドルを、パスごとに1回だけ読み込んで使い回す
    int LoadNewsImage(const std::string& path)
    {
        if (path.empty()) return -1;

        static std::unordered_map<std::string, int> cache;
        auto it = cache.find(path);
        if (it != cache.end()) return it->second;

        const int handle = LoadGraph(path.c_str());
        cache[path] = handle;   // 読み込めなかった場合(-1)も記録して、毎回読み直さない
        return handle;
    }
}

NewsViewer::NewsViewer()
    : m_open(false)
    , m_timer(0.0f)
    , m_imageHandle(-1)
{
}

void NewsViewer::Open(const NewsData& news)
{
    m_title = news.title;
    m_date = news.date;
    m_imageHandle = LoadNewsImage(news.imagePath);

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

    // 内容を確認したあと、決定キーでゲーム画面へ戻る
    if (m_timer >= MIN_SHOW_TIME && input.IsTrigger(DECISION_ACTION))
    {
        Close();
    }
}

void NewsViewer::Draw() const
{
    if (!m_open) return;

    const float fade = (std::min)(m_timer / FADE_IN_TIME, 1.0f);

    // 背景を暗くする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(180 * fade));
    DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);

    // 新聞(画像は縦横比を保って、枠に収める。NewsMenu と同じ考え方)
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * fade));

    int w = 0;
    int h = 0;
    if (m_imageHandle >= 0) GetGraphSize(m_imageHandle, &w, &h);

    if (m_imageHandle >= 0 && w > 0 && h > 0)
    {
        const float sx = static_cast<float>(IMAGE_AREA_W) / w;
        const float sy = static_cast<float>(IMAGE_AREA_H) / h;
        const float scale = (sx < sy) ? sx : sy;

        const int dw = static_cast<int>(w * scale);
        const int dh = static_cast<int>(h * scale);
        const int x = (SCREEN_W - dw) / 2;
        const int y = (SCREEN_H - dh) / 2 - 20;

        DrawExtendGraph(x, y, x + dw, y + dh, m_imageHandle, TRUE);
    }
    else
    {
        // 画像がない間の仮表示: 紙の四角に、見出しと日付を表示する
        const int left = (SCREEN_W - IMAGE_AREA_W) / 2;
        const int top = (SCREEN_H - IMAGE_AREA_H) / 2 - 20;
        DrawBox(left, top, left + IMAGE_AREA_W, top + IMAGE_AREA_H, GetColor(238, 228, 196), TRUE);
        DrawBox(left, top, left + IMAGE_AREA_W, top + IMAGE_AREA_H, GetColor(60, 45, 30), FALSE);
        DrawString(left + 40, top + 40, m_title.c_str(), GetColor(40, 30, 20));
        DrawString(left + 40, top + 80, m_date.c_str(), GetColor(80, 60, 40));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 閉じる案内(閉じられるようになったら表示)
    if (m_timer >= MIN_SHOW_TIME)
    {
        DrawString(SCREEN_W / 2 - 80, SCREEN_H - 50, "決定キーで閉じる", GetColor(255, 255, 255));
    }
}