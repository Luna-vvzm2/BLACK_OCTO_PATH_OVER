#pragma once

class SubMenu;

class SubMenuStatusPage
{
public:
    explicit SubMenuStatusPage(SubMenu* owner);
    ~SubMenuStatusPage() = default;

    void Initialize();
    void Update(float deltaTime);
    void Draw();

private:
    SubMenu* m_owner = nullptr;
};