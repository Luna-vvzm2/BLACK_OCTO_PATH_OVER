#pragma once

#include "NewsData.h"

#include <string>

class Input;

// 新聞の表示画面(ゲーム画面の上に重ねて、新聞画像を大きく表示する)。
// 表示中は PlayScene が通常の更新を止める。内容を確認したあと、決定キーで閉じてゲーム画面へ戻る。
class NewsViewer {
public:
    NewsViewer();

    void Open(const NewsData& news);    // 新聞を表示する
    void Close();
    bool IsOpen() const { return m_open; }

    // 表示中に毎フレーム呼ぶ。決定キーが押されたら閉じる。
    void Update(float deltaTime, const Input& input);

    // 画面の一番上に描画する。
    void Draw() const;

private:
    bool m_open;
    float m_timer;            // 開いてからの経過時間(フェードイン・誤入力防止用)
    std::string m_title;
    std::string m_date;
    int m_imageHandle;        // 新聞画像のハンドル(-1 = 画像なし)
};
