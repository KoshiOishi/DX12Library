#pragma once
#include "Scene.h"
#include "Object3D.h"
#include "Sprite.h"
#include "GamePlay.h"

class GamePlay :
	public Scene
{
private:
	Sprite sprite1;
	Sprite sprite2;

	Object3D obj1;
	Object3D obj2;

	Model model1;
	Model model2;


public:
	GamePlay();
	~GamePlay();
	void Initialize() override;
	void Update() override;
	void Draw() override;
};

