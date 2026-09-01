#pragma once
#include "UIActor.h"
#include <string>

class PlayerEntity;
class Renderer;

class MoneyUI : public UIActor {
public:
    enum class Anchor {
        TopLeft,
        TopRight
    };
    MoneyUI(Scene* scene, PlayerEntity* player, const std::string& coinImagePath = "assets/images/uies/money.png");
    virtual ~MoneyUI();

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    // スクリーン座標で位置指定（左上基準）
    void SetPosition(float x, float y);

    // 画像の描画サイズ（px）
    void SetImageSize(float w, float h);

    // 画像のスクリーン単位オフセット（px）。正で右/下、負で左/上。
    void SetImageOffset(float offsetX, float offsetY);

    // テキストオフセット（画像の右端からの距離）
    void SetTextOffset(float offsetX) { m_textOffsetX = offsetX; }

    // 表示制御: seconds > 0 でその秒数表示、seconds <= 0 で無期限表示
    void ShowFor(float seconds);

    void SetAnchorTopRight(float marginRight, float marginTop, float gap = 8.0f) {
        m_anchor = Anchor::TopRight;
        m_marginRight = marginRight;
        m_marginTop = marginTop;
        m_gap = gap;
    }

private:
    PlayerEntity* m_player = nullptr;

    std::string m_coinImagePath;
    int m_coinHandle = -1; // DxLib ハンドル（LoadGraph の戻り値）

    // スクリーン基準位置（左上基準）
    float m_screenX = 0.0f;
    float m_screenY = 0.0f;

    // 描画サイズ
    float m_imageW = 32.0f;
    float m_imageH = 32.0f;

    // 画像オフセット（スクリーン単位）
    float m_imageOffsetX = 0.0f;
    float m_imageOffsetY = 0.0f;

    // テキストオフセット（px）
    float m_textOffsetX = 36.0f;

    int m_fontSize = 20;

    // 表示管理
    bool m_visible = false;
    float m_showTimer = 0.0f; // >0: 残り秒数, ==0: 非表示, <0: 無期限表示

    Anchor m_anchor = Anchor::TopLeft; // デフォルトは左上
    float m_marginRight = 20.0f;
    float m_marginTop = 10.0f;
    float m_gap = 8.0f; // 画像と数字の間隔（px）
};