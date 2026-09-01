#pragma once

#include "UIActor.h"

class SkillMenuActor : public UIActor
{
public:

    SkillMenuActor(Scene* scene);

    void Update(float deltaTime) override;
    void Draw() override;

private:

    int m_selectIndex;
};
