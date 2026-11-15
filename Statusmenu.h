#pragma once
#include "MenuPage.h"

class StatusMenu : public MenuPage
{
public:
    explicit StatusMenu(Menu* owner);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

protected:
    void ClampCursor() override;
};