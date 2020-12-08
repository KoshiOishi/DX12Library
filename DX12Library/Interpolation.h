#pragma once

class Interpolation
{
private:
	float second = 1.0f;
	float timeRate = 0.0f;
	bool isStart = false;

public:
	/// <summary>
	/// タイムレートの更新
	/// </summary>
	void Update(float fps);

	/// <summary>
	/// タイマーをスタートさせる
	/// </summary>
	/// <param name="second">補間秒数</param>
	void Start(float second);

	/// <summary>
	/// 線形補間（float）
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="timeRate">時間経過（0～1）</param>
	/// <returns>補間された値</returns>
	static float Leap(const float start, const float end, const float timeRate);

	/// <summary>
	/// イーズイン補間（float）
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="timeRate">時間経過（0～1）</param>
	/// <returns>補間された値</returns>
	static float EaseIn(const float start, const float end, const float timeRate);

	/// <summary>
	/// イーズアウト補間（float）
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="timeRate">時間経過（0～1）</param>
	/// <returns>補間された値</returns>
	static float EaseOut(const float start, const float end, const float timeRate);

	/// <summary>
	/// イーズインアウト補間（float）
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="timeRate">時間経過（0～1）</param>
	/// <returns>補間された値</returns>
	static float EaseInOut(const float start, const float end, const float timeRate);


	float GetTimeRate() { return timeRate; }
};

