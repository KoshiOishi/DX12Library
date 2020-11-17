#pragma once
#include <string>

class Scene
{
	using string = std::string;
protected:
	string nextScene;
	string sceneName;

public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual string GetNextScene();
	virtual string GetSceneName();
};

