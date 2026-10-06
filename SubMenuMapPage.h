#pragma once
#include "Vector2d.h"


class SubMenu;

class SubMenuMapPage
{
public:
    explicit SubMenuMapPage(SubMenu* owner);
    ~SubMenuMapPage() = default;

    void Initialize();
    void Update(float deltaTime);
    void Draw();

private:
    SubMenu* m_owner = nullptr;

    // マップ操作パラメータ
    float m_zoom = 1.0f;       // 拡大率
    Vector2d m_scrollOffset;   // スクロールオフセット位置
};