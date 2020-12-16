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
private://�G�C���A�X
//Microsoft::WRL::���ȗ�
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	//DirectX::���ȗ�
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

