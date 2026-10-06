#pragma once
#include "UIActor.h"
#include "NewsData.h"
#include <vector>
#include <memory>

class PlayerEntity;
class SubMenuMapPage;     // 追加
class SubMenuStatusPage;
class SubMenuNewsPage;

class SubMenu : public UIActor
{
public:
    enum class Tab
    {
        Map = 0,    // マップ(初期表示)
        Status,     // ステータス
        News,       // 新聞
        Max
    };

    explicit SubMenu(Scene* scene);
    ~SubMenu() override;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // 開閉
    void Open();
    void Close();
    void Toggle();
    bool IsOpen() const { return m_isOpen; }

    // 新聞入手処理(宝箱開封時などに呼び出す)
    void UnlockNews(int newsId);

    // データアクセサ
    const std::vector<NewsData>& GetNewsList() const { return m_newsList; }
    PlayerEntity* GetPlayer() const { return m_player; }

private:
    void UpdateTabInput();
    void DrawHeader();

private:
    bool m_isOpen = false;
    Tab m_currentTab = Tab::Map;

    PlayerEntity* m_player = nullptr;
    std::vector<NewsData> m_newsList;

    // 各タブ用ページクラス(.cpp の初期化子リストと同じ順序にする)
    std::unique_ptr<SubMenuMapPage>    m_mapPage;     // 追加
    std::unique_ptr<SubMenuStatusPage> m_statusPage;
    std::unique_ptr<SubMenuNewsPage>   m_newsPage;

    // キーリピート対策などのタイマー
    float m_tabSwitchTimer = 0.0f;
};