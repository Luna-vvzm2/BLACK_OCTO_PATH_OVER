#include "SkillMenuActor.h"

#include <DxLib.h>

SkillMenuActor::SkillMenuActor(Scene* scene)
    : UIActor(scene)
    , m_selectIndex(0)
{
}

void SkillMenuActor::Update(float deltaTime)
{
    UIActor::Update(deltaTime);
}

void SkillMenuActor::Draw()
{
    printf("SkillMenu Draw\n");

    DrawString(
        100,
        100,
        "SKILL MENU",
        GetColor(255, 255, 255)
    );
}