#pragma once
#include "Scene.h"
#include "Object3D.h"
#include "Sprite.h"
#include "GamePlay.h"
#include "CollisionPrimitive.h"
#include "Interpolation.h"
#include "Light.h"

class GamePlay :
	public Scene
{
private://エイリアス
//Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	//DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

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

	Light light;

public:
	GamePlay();
	~GamePlay();
	void Initialize() override;
	void Update() override;
	void Draw() override;
};

