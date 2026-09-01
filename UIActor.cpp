#include "UIActor.h"
#include "Scene.h"

UIActor::UIActor(Scene* scene)
    : Actor(scene)
    , m_posX(0.0f)
    , m_posY(0.0f)
{
    SetName("UIActor");
}

void UIActor::Update(float deltaTime)
{
    Actor::Update(deltaTime);
    // ※ 必要な UI 更新があればここに書く
}

void UIActor::Draw()
{
    //  本来ここで UI 用の描画処理（テキスト・スプライトなど）を書く
    //  UIActor 自体は描画を持たないので空処理にしておき、
    //  派生クラスで実装する

    Actor::Draw();
}