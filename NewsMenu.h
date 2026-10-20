#pragma once
#include "MenuPage.h"

class NewsMenu : public MenuPage
{
public:
    explicit NewsMenu(Menu* owner);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // “üèó‹µ‚Ìİ’è (Œã‚ÅSaveData‚Æ˜AŒg—\’è)
    void SetOwned(int index, bool owned);

    static constexpr int NewsCount = 6;

protected:
    void ClampCursor() override;

private:
    bool m_owned[NewsCount] = {};
};
