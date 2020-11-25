#pragma once
#include <string>
#include <vector>
#include "Scene.h"

class SceneManager
{
	using string = std::string;
	template <typename T>
	using vector = std::vector<T>;

private:
	static string nowScene;
	static vector<Scene*> scenes;
	static int sceneNum;

public:
	static void AddScene(Scene* scene);
	static void SetScene(string sceneName);

	static void Initialize();
	static void Update();
	static void Draw();
};

