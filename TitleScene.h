#pragma once
#include "Scene.h"
#include "Vector2d.h"

class TitleScene : public Scene
{
public:
	//	コンストラクタ
	TitleScene(class Game* game);
	//	デストラクタ
	~TitleScene() override = default;

	bool Init() override;

	void Update(float deltaTime) override;

	void Draw() override;

private:

	Vector2d titlePos;
};

