#pragma once
#include <string>
#include <vector>

// 新聞データの構造体（画像表示用）
struct NewsData
{
    int id = 0;                  // ID (1~6)
    std::string title = "";      // タイトル（画面リスト用）
    std::string date = "";       // 日付
    int stageNo = 0;             // 入手ステージ
    std::string imagePath = "";  // 表示する新聞画像のファイルパス
    bool unlocked = false;       // 所持・解放フラグ
};

// 新聞の初期データ一覧を取得する関数
inline std::vector<NewsData> GetDefaultNewsList()
{
    std::vector<NewsData> list;

    NewsData n1;
    n1.id = 1;
    n1.title = "金城失踪から早10日";
    n1.date = "2000年5月23日";
    n1.stageNo = 1;
    n1.imagePath = "assets/images/uies/news_1.png";
    n1.unlocked = true;   // 最初から所持
    list.push_back(n1);

    NewsData n2;
    n2.id = 2;
    n2.title = "黒い怪物にご用心";
    n2.date = "1987年10月8日";
    n2.stageNo = 1;
    n2.imagePath = "assets/images/uies/news_2.png";
    n2.unlocked = false;
    list.push_back(n2);

    return list;
}