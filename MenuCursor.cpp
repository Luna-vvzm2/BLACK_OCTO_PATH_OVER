#include "MenuCursor.h"

int MenuCursor::GetCategory() const
{
    return m_category;
}

void MenuCursor::SetCategory(int category)
{
    m_category = category;
}

int MenuCursor::GetIndex() const
{
    return m_index;
}

void MenuCursor::SetIndex(int index)
{
    m_index = index;
}

void MenuCursor::SetPosition(
    int category,
    int index
)
{
    m_category = category;
    m_index = index;
}

void MenuCursor::ClampCategory(int maxCategory)
{
    if (m_category < 0)
        m_category = 0;

    if (m_category > maxCategory)
        m_category = maxCategory;
}

void MenuCursor::ClampIndex(int maxIndex)
{
    if (m_index < 0)
        m_index = 0;

    if (m_index > maxIndex)
        m_index = maxIndex;
}

bool MenuCursor::IsRepeat(
    Action action,
    const Input& input,
    float deltaTime)
{
    float* timer = nullptr;

    switch (action)
    {
    case Action::LEFT:
        timer = &m_leftTimer;
        break;

    case Action::RIGHT:
        timer = &m_rightTimer;
        break;

    case Action::UP:
        timer = &m_upTimer;
        break;

    case Action::DOWN:
        timer = &m_downTimer;
        break;

    default:
        return false;
    }

    if (input.IsTrigger(action))
    {
        *timer = 0.0f;
        return true;
    }

    if (input.IsDown(action))
    {
        *timer += deltaTime;

        if (*timer >= RepeatDelay)
        {
            *timer -= RepeatInterval;
            return true;
        }
    }
    else
    {
        *timer = 0.0f;
    }

    return false;
}
