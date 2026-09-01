#pragma once

#include <vector>
#include "MenuPage.h"
#include "SkillData.h"
#include "Vector2d.h"

class SkillMenu : public MenuPage
{
public:

    explicit SkillMenu(Menu* owner);

    void Initialize();
    void Update(float deltaTime) override;
    void Draw() override;
    void ClampCursor() override;

private:

    struct SkillCategory
    {
        std::string name;

        std::vector<SkillData>* skills;

        bool grid = false;

        int columns = 0;

        int titleY = 0;

        int skillY = 0;
    };

    std::vector<SkillCategory> m_categories;
    std::vector<SkillData> m_ninjutsu;
    std::vector<SkillData> m_ninpou;
    std::vector<SkillData> m_ningi;
    std::vector<SkillData> m_combat;

    Vector2d GetSkillPosition(
        const SkillCategory& category,
        int index
    ) const;

    int m_lockedSkillIcon = -1;

    SkillData* GetSelectedSkill();

    void DrawSkillSlot(
        int x,
        int y,
        const SkillData& skill) const;

    float m_verticalRepeatTimer = 0.0f;

    static constexpr float VerticalRepeatDelay = 0.25f;
    static constexpr float VerticalRepeatInterval = 0.08f;

    void MoveUp();
    void MoveDown();

};