#include "Interpolation.h"

void Interpolation::Update(float fps)
{
	if (!isStart) return;

	float add = second / fps;
	timeRate += add;
	if (timeRate > 1.0f)
	{
		timeRate = 1.0f;
		isStart = false;
	}
}

void Interpolation::Start(float second)
{
	if (!isStart)
	{
		isStart = true;
		timeRate = 0;
		this->second = second;
	}
}

float Interpolation::Leap(const float start, const float end, const float timeRate)
{
	return start * (1.0f - timeRate) + end * timeRate;
}

float Interpolation::EaseIn(const float start, const float end, const float timeRate)
{
	float f = timeRate * timeRate;
	return start * (1.0f - f) + end * f;
}

float Interpolation::EaseOut(const float start, const float end, const float timeRate)
{
	float f = timeRate * (2 - timeRate);
	return start * (1.0f - f) + end * f;
}

float Interpolation::EaseInOut(const float start, const float end, const float timeRate)
{
	float f = timeRate * timeRate * (3 - 2 * timeRate);
	return start * (1.0f - f) + end * f;
}
