#include <assert.h>
#include "Input.h"
#include "SceneManager.h"

//静的メンバ変数の実体
SceneManager::string SceneManager::nowScene;
SceneManager::vector<Scene*> SceneManager::scenes;
int SceneManager::sceneNum;

void SceneManager::AddScene(Scene * scene)
{
	scenes.push_back(scene);
}

void SceneManager::SetScene(string sceneName)
{
	nowScene = sceneName;
	for (int i = 0; i < scenes.size(); i++)
	{
		//現在シーンのみ適用
		if (scenes[i]->GetSceneName() != nowScene) continue;

		sceneNum = i;
		Initialize();
		return;
	}
}

void SceneManager::Initialize()
{
	scenes[sceneNum]->Initialize();
}

void SceneManager::Update()
{
	Input::Update();
	scenes[sceneNum]->Update();
}

void SceneManager::Draw()
{
	scenes[sceneNum]->Draw();
}
