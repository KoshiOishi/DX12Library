#pragma once
#include "Scene.h"
#include "Object3D.h"
#include "Sprite.h"
#include "GamePlay.h"
#include "CollisionPrimitive.h"
#include "Interpolation.h"

class GamePlay :
	public Scene
{
private:
	Sprite sprite1;
	Sprite sprite2;

	Object3D obj1;
	Object3D obj2;
	Object3D obj3;

	Model model1;
	Model model2;

	Sphere sphere1;
	Sphere sphere2;

	Interpolation ip;

public:
	GamePlay();
	~GamePlay();
	void Initialize() override;
	void Update() override;
	void Draw() override;
};

