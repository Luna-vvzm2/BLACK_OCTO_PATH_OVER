#include "SkillMenu.h"

#include "PlayScene.h"
#include "Game.h"

#include <DxLib.h>
#include "MenuCursor.h"

SkillMenu::SkillMenu(Menu* owner)
    : MenuPage(owner)
{
}

void SkillMenu::DrawSkillSlot(
    int x,
    int y,
    const SkillData& skill
) const
{
    const int size = 60;

    int handle =
        skill.unlocked ?
        skill.iconHandle :
        m_owner->GetLockedSkillIcon();

    if (handle >= 0)
    {
        DrawExtendGraph(
            x,
            y,
            x + size,
            y + size,
            handle,
            TRUE
        );
    }
    else
    {
        DrawBox(
            x,
            y,
            x + size,
            y + size,
            GetColor(255, 255, 255),
            FALSE
        );
    }
}

void SkillMenu::Initialize()
{
    m_ninjutsu =
    {
        {"îEèp1","ãZê‡ñæóp"},
        {"îEèp2","test"},
        {"îEèp3","test"},
        {"îEèp4","test"}
    };

    m_ninpou =
    {
        {"îEñ@1","test"},
        {"îEñ@2","test"},
        {"îEñ@3","test"},
        {"îEñ@4","test"},
        {"îEñ@5","test"},
        {"îEñ@6","test"},
        {"îEñ@7","test"},
        {"îEñ@8","test"}
    };

    m_ningi =
    {
        {"îEãZ1","test"},
        {"îEãZ2","test"},
        {"îEãZ3","test"},
        {"îEãZ4","test"},
        {"îEãZ5","test"},
        {"îEãZ6","test"},
        {"îEãZ7","test"}
    };

    m_combat =
    {
        {"êÌì¨ãZ1","test"},
        {"êÌì¨ãZ2","test"},
        {"êÌì¨ãZ3","test"},
        {"êÌì¨ãZ4","test"},
        {"êÌì¨ãZ5","test"},
        {"êÌì¨ãZ6","test"},
        {"êÌì¨ãZ7","test"},
        {"êÌì¨ãZ8","test"},
        {"êÌì¨ãZ9","test"},

        {"êÌì¨ãZ10","test"},
        {"êÌì¨ãZ11","test"},
        {"êÌì¨ãZ12","test"},
        {"êÌì¨ãZ13","test"},
        {"êÌì¨ãZ14","test"},
        {"êÌì¨ãZ15","test"},
        {"êÌì¨ãZ16","test"},
        {"êÌì¨ãZ17","test"}
    };

    m_categories =
    {
        {"îEèp",&m_ninjutsu,false,4,120,160},

        {"îEñ@",&m_ninpou,false,8,250,290},

        {"îEãZ",&m_ningi,false,7,380,420},

        {"êÌì¨ãZ",&m_combat,true,9,510,550}
    };
}

void SkillMenu::Draw()
{

    auto& cursor = m_owner->GetCursor();

    const int startX = 50;
    const int size = 60;
    const int gap = 15;

    for (const auto& category : m_categories)
    {
        DrawString(
            startX,
            category.titleY,
            category.name.c_str(),
            GetColor(255, 255, 255)
        );

        for (int i = 0;
            i < category.skills->size();
            i++)
        {
            Vector2d pos =
                GetSkillPosition(category, i);

            DrawSkillSlot(
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                (*category.skills)[i]
            );
        }
    }

    //----------------------------------
    // ÉJÅ[É\Éã
    //----------------------------------
    if (m_owner->GetCursorArea() == Menu::MenuCursorArea::SkillList)
    {
        if (cursor.GetCategory() < 0 ||
            cursor.GetCategory() >= static_cast<int>(m_categories.size()))
        {
            return;
        }

        auto& category =
            m_categories[cursor.GetCategory()];

        if (cursor.GetIndex() < 0 ||
            cursor.GetIndex() >= static_cast<int>(category.skills->size()))
        {
            return;
        }

        Vector2d pos =
            GetSkillPosition(
                category,
                cursor.GetIndex()
            );

        DrawBox(
            static_cast<int>(pos.x) - 5,
            static_cast<int>(pos.y) - 5,
            static_cast<int>(pos.x) + size + 5,
            static_cast<int>(pos.y) + size + 5,
            GetColor(255, 0, 0),
            FALSE
        );

        //----------------------------------
        // ê‡ñæóì
        //----------------------------------

        DrawBox(
            800,
            150,
            1200,
            400,
            GetColor(80, 80, 80),
            TRUE
        );

        SkillData* skill =
            GetSelectedSkill();

        if (skill)
        {
            DrawString(
                975,
                430,
                skill->name.c_str(),
                GetColor(255, 255, 255)
            );

            DrawString(
                975,
                600,
                skill->description.c_str(),
                GetColor(255, 255, 255)
            );
        }
    }
}

void SkillMenu::Update(float deltaTime)
{
    if (m_owner->GetCursorArea() != Menu::MenuCursorArea::SkillList)
        return;

    auto& cursor = m_owner->GetCursor();

    const Input& input =
        m_owner->GetGame()->GetInput();

    //----------------------------------
    // èââÒì¸óÕ
    //----------------------------------

    if (cursor.IsRepeat(Action::LEFT, input, deltaTime))
    {
        cursor.SetIndex(cursor.GetIndex() - 1);
    }

    if (cursor.IsRepeat(Action::RIGHT, input, deltaTime))
    {
        cursor.SetIndex(cursor.GetIndex() + 1);
    }

    if (cursor.IsRepeat(Action::UP, input, deltaTime))
    {
        MoveUp();
    }

    if (cursor.IsRepeat(Action::DOWN, input, deltaTime))
    {
        MoveDown();
    }

    cursor.ClampCategory(
        static_cast<int>(m_categories.size()) - 1
    );

    ClampCursor();
}

void SkillMenu::MoveDown()
{
    auto& cursor = m_owner->GetCursor();

    if (cursor.GetCategory() == 3)
    {
        if (cursor.GetIndex() < 9)
        {
            cursor.SetIndex(cursor.GetIndex() + 9);
        }
    }
    else
    {
        cursor.SetCategory(cursor.GetCategory() + 1);
    }
}

void SkillMenu::MoveUp()
{
    auto& cursor = m_owner->GetCursor();

    if (cursor.GetCategory() == 3)
    {
        if (cursor.GetIndex() >= 9)
        {
            cursor.SetIndex(cursor.GetIndex() - 9);
        }
        else
        {
            cursor.SetCategory(cursor.GetCategory() - 1);
        }
    }
    else
    {
        if (cursor.GetCategory() == 0)
        {
            m_owner->SetCursorArea(Menu::MenuCursorArea::Tab);
            m_verticalRepeatTimer = 0.0f;
        }
        else
        {
            cursor.SetCategory(cursor.GetCategory() - 1);
        }
    }
}

void SkillMenu::ClampCursor()
{
    auto& cursor = m_owner->GetCursor();

    cursor.ClampCategory(
        static_cast<int>(m_categories.size()) - 1
    );

    auto& category =
        m_categories[cursor.GetCategory()];

    cursor.ClampIndex(
        static_cast<int>(category.skills->size()) - 1
    );
}

SkillData* SkillMenu::GetSelectedSkill()
{
    auto& cursor = m_owner->GetCursor();

    int category = cursor.GetCategory();

    if (category < 0 ||
        category >= m_categories.size())
    {
        return nullptr;
    }

    auto& skills =
        *m_categories[category].skills;

    int index =
        cursor.GetIndex();

    if (index < 0 ||
        index >= skills.size())
    {
        return nullptr;
    }

    return &skills[index];
}

Vector2d SkillMenu::GetSkillPosition(
    const SkillCategory& category,
    int index
) const
{
    const int startX = 50;
    const int size = 60;
    const int gap = 15;

    int columns =
        category.grid
        ? category.columns
        : static_cast<int>(category.skills->size());

    int x =
        startX
        + (index % columns) * (size + gap);

    int y =
        category.skillY
        + (index / columns) * (size + gap);

    return Vector2d(
        static_cast<float>(x),
        static_cast<float>(y)
    );
}