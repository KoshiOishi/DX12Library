#include <assert.h>
#include "SceneManager.h"

//�ÓI�����o�ϐ��̎���
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
		//���݃V�[���̂ݓK�p
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Initialize();
		return;
	}

	//�ǂ̃V�[�����Ă΂�Ȃ�������x��
	assert(0);
}

void SceneManager::Update()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		//���݃V�[���̂ݓK�p
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Update();
		return;
	}

	//�ǂ̃V�[�����Ă΂�Ȃ�������x��
	assert(0);
}

void SceneManager::Draw()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		//���݃V�[���̂ݓK�p
		if (scenes[i]->GetSceneName() != nowScene) continue;

		scenes[i]->Draw();
		return;
	}

	//�ǂ̃V�[�����Ă΂�Ȃ�������x��
	assert(0);
}
