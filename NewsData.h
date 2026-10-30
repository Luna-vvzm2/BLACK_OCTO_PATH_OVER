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

    NewsData n3;
    n3.id = 3;
    n3.title = "話題のルーキー『金城清誠』";
    n3.date = "1995年7月5日";
    n3.imagePath = "assets/images/uies/news_3.png";
    list.push_back(n3);

    NewsData n4;
    n4.id = 4;
    n4.title = "大江山近辺から国民を保護";
    n4.date = "1970年6月16日";
    n4.imagePath = "assets/images/uies/news_4.png";
    list.push_back(n4);

    NewsData n5;
    n5.id = 5;
    n5.title = "貢献者への勲章授与式";
    n5.date = "1970年7月1日";
    n5.imagePath = "assets/images/uies/news_5.png";
    list.push_back(n5);

    NewsData n6;
    n6.id = 6;
    n6.title = "噂の『影の魔術師』の行方";
    n6.date = "1901年2月11日";
    n6.imagePath = "assets/images/uies/news_6.png";
    list.push_back(n6);

    return list;
}