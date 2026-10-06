#include "SubMenuStatusPage.h"
#include "SubMenu.h"
#include "PlayerEntity.h"
#include "Scene.h"
#include "Game.h"
#include "Renderer.h"


SubMenuStatusPage::SubMenuStatusPage(SubMenu* owner)
    : m_owner(owner)
{
}

void SubMenuStatusPage::Initialize()
{
}

void SubMenuStatusPage::Update(float deltaTime)
{
}

void SubMenuStatusPage::Draw()
{
    if (!m_owner) return;

    Renderer* renderer = m_owner->GetScene()->GetGame()->GetRenderer();

    const std::string& font =
        m_owner->GetScene()->GetGame()->GatDebugFont();

    PlayerEntity* player = m_owner->GetPlayer();

    // 枠線の設定
    float leftX = 120.0f;
    float startY = 140.0f;
    float width = 1040.0f;
    float height = 480.0f;

    // ステータス表示領域の背景
    DrawBox(
        static_cast<int>(leftX),
        static_cast<int>(startY),
        static_cast<int>(leftX + width),
        static_cast<int>(startY + height),
        GetColor(25, 25, 30),
        TRUE
    );

    DrawBox(
        static_cast<int>(leftX),
        static_cast<int>(startY),
        static_cast<int>(leftX + width),
        static_cast<int>(startY + height),
        GetColor(100, 100, 120),
        FALSE
    );

    renderer->DrawTextL(
        Vector2d(leftX + 30.0f, startY + 25.0f),
        "【 キャラクターステータス 】",
        Color(255, 215, 0),
        font,
        24,
        false
    );

    // プレイヤーデータの取得・表示（仮値/実データ連携）
    int hp = 100;
    int maxHp = 100;
    int kage = 100;
    int maxKage = 100;
    int kunai = 0;
    int item = 0;

    if (player)
    {
        // PlayerEntity の実装に合わせて実際の数値を取得
        // hp = player->GetHP();
        // maxHp = player->GetMaxHP();
    }

    // ステータス項目テキスト描画
    float textX = leftX + 50.0f;
    float currY = startY + 80.0f;
    float lineGap = 40.0f;

    // 1. 体力 (HP)
    std::string hpStr =
        "HP: " + std::to_string(hp) + " / " + std::to_string(maxHp);

    renderer->DrawTextL(
        Vector2d(textX, currY),
        hpStr,
        Color(255, 255, 255),
        font,
        20,
        false
    );

    currY += lineGap;

    // 2. 影ゲージ
    std::string kageStr =
        "影ゲージ: " + std::to_string(kage) + " / " + std::to_string(maxKage);

    renderer->DrawTextL(
        Vector2d(textX, currY),
        kageStr,
        Color(200, 200, 255),
        font,
        20,
        false
    );

    currY += lineGap;

    // 3. クナイ所持数
    std::string kunaiStr =
        "クナイ所持数: " + std::to_string(kunai);

    renderer->DrawTextL(
        Vector2d(textX, currY),
        kunaiStr,
        Color(255, 255, 255),
        font,
        20,
        false
    );

    currY += lineGap;

    // 4. 回復アイテム所持数
    std::string itemStr =
        "回復アイテム: " + std::to_string(item) + " / 4";

    renderer->DrawTextL(
        Vector2d(textX, currY),
        itemStr,
        Color(255, 255, 255),
        font,
        20,
        false
    );

    currY += lineGap + 20.0f;

    // 5. 所持忍術一覧
    renderer->DrawTextL(
        Vector2d(textX, currY),
        "【 忍術 】",
        Color(255, 215, 0),
        font,
        20,
        false
    );

    currY += 35.0f;

    const char* ninjutsuList[] = { "蛸", "虎", "蛙", "鯱" };

    for (int i = 0; i < 4; ++i)
    {
        float iconX = textX + i * 220.0f;

        DrawBox(
            static_cast<int>(iconX),
            static_cast<int>(currY),
            static_cast<int>(iconX + 200.0f),
            static_cast<int>(currY + 80.0f),
            GetColor(45, 45, 55),
            TRUE
        );

        DrawBox(
            static_cast<int>(iconX),
            static_cast<int>(currY),
            static_cast<int>(iconX + 200.0f),
            static_cast<int>(currY + 80.0f),
            GetColor(90, 90, 100),
            FALSE
        );

        renderer->DrawTextC(
            Vector2d(iconX + 100.0f, currY + 28.0f),
            ninjutsuList[i],
            Color(180, 180, 180),
            font,
            18,
            false
        );
    }
}