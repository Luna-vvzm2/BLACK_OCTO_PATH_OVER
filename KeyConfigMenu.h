#pragma once

#include "MenuPage.h"
#include "Input.h"

#include <vector>

class KeyConfigMenu : public MenuPage
{
public:
    explicit KeyConfigMenu(Menu* owner);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // ÉLÅ[äÑÇËìñÇƒíÜÇ©Ç«Ç§Ç©
    bool IsWaitingForKey() const
    {
        return m_waitingForKey;
    }

private:
    struct BindingRow
    {
        Action action;
        const char* label;
    };

    static const std::vector<BindingRow>& GetRows();

    int m_selectedIndex = 0;
    bool m_waitingForKey = false;

    void ClampCursor() override;
};