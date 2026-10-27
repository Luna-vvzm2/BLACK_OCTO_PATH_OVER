#pragma once
#include "MenuPage.h"

class NewsMenu : public MenuPage
{
public:
    explicit NewsMenu(Menu* owner);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // “üèó‹µ‚Ìİ’è
    void SetOwned(int index, bool owned);
    bool IsOwned(int index) const
    {
        return index >= 0 && index < NewsCount && m_owned[index];
    }

    static constexpr int NewsCount = 6;

protected:
    void ClampCursor() override;

private:
    bool m_owned[NewsCount] = {};
    int  m_imageHandle[NewsCount];   // ‰æ‘œƒnƒ“ƒhƒ‹ (-1 = ‰æ‘œ‚È‚µ)
};
