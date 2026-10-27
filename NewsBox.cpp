#include "NewsBox.h"
#include "PlayScene.h"

namespace
{
    // 開いてから新聞を表示するまでの時間。粉砕の演出(フェード)を少し見せてから表示する。
    // TreasureBoxEntity は、開いてから約1.0秒(待ち0.2秒 + フェード0.8秒)で消えるので、それより短くする。
    constexpr float NEWS_SHOW_DELAY = 0.4f;
}

NewsBoxEntity::NewsBoxEntity(Scene* scene, const Vector2d& pos, int newsId)
    : TreasureBoxEntity(scene, pos)
    , m_newsId(newsId)
    , m_broken(false)
    , m_shown(false)
    , m_showTimer(0.0f)
{
}

bool NewsBoxEntity::Init()
{
    if (!TreasureBoxEntity::Init())
        return false;

    // 通常の宝箱が落とす回復・コインは、新聞の宝箱では落とさない
    m_dropTable.clear();
    return true;
}

void NewsBoxEntity::OnDead()
{
    // 開いた(粉砕が始まった)ことを記録する。新聞の表示は Update で少し遅らせて行う。
    m_broken = true;

    TreasureBoxEntity::OnDead();   // 粉砕(フェード)を開始する。ドロップ表は空なので、アイテムは出ない。
}

void NewsBoxEntity::Update(float deltaTime)
{
    TreasureBoxEntity::Update(deltaTime);

    if (!m_broken || m_shown) return;

    m_showTimer += deltaTime;
    if (m_showTimer >= NEWS_SHOW_DELAY)
    {
        m_shown = true;

        // 新聞を入手して表示する(PlayScene が、ゲームを止めて新聞を表示する)
        PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
        if (playScene)
        {
            playScene->ShowNews(m_newsId);
        }
    }
}