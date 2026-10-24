#pragma once
#include "MenuPage.h"

class MapMenu : public MenuPage
{
public:
    explicit MapMenu(Menu* owner);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

protected:
    void ClampCursor() override;
};