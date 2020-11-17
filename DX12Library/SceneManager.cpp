#include <assert.h>
#include "SceneManager.h"

//静的メンバ変数の実体
SceneManager::string SceneManager::nowScene;
SceneManager::vector<Scene*> SceneManager::scenes;

void SceneManager::AddScene(Scene * scene)
{
	scenes.push_back(scene);
}

void SceneManager::SetScene(string sceneName)
{
	nowScene = sceneName;
	Initialize();
}

void SceneManager::Initialize()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		//現在シーンのみ適用
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Initialize();
		return;
	}

	//どのシーンも呼ばれなかったら警告
	assert(0);
}

void SceneManager::Update()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		//現在シーンのみ適用
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Update();
		return;
	}

	//どのシーンも呼ばれなかったら警告
	assert(0);
}

void SceneManager::Draw()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		//現在シーンのみ適用
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Draw();
		return;
	}

	//どのシーンも呼ばれなかったら警告
	assert(0);
}
