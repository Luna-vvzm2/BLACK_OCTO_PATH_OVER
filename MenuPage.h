#pragma once

class Menu;

class MenuPage
{
public:
    explicit MenuPage(Menu* owner);
    virtual ~MenuPage() = default;

    virtual void Initialize() {}
    virtual void Update(float deltaTime) {}
    virtual void Draw() = 0;

protected:
    virtual void ClampCursor() = 0;

    Menu* m_owner;
};
