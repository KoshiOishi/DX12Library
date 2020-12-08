#pragma once

class Interpolation
{
private:
	float second = 1.0f;
	float timeRate = 0.0f;
	bool isStart = false;

public:
	/// <summary>
	/// �^�C�����[�g�̍X�V
	/// </summary>
	void Update(float fps);

	/// <summary>
	/// �^�C�}�[���X�^�[�g������
	/// </summary>
	/// <param name="second">��ԕb��</param>
	void Start(float second);

	/// <summary>
	/// ���`��ԁifloat�j
	/// </summary>
	/// <param name="start">�J�n�l</param>
	/// <param name="end">�I���l</param>
	/// <param name="timeRate">���Ԍo�߁i0�`1�j</param>
	/// <returns>��Ԃ��ꂽ�l</returns>
	static float Leap(const float start, const float end, const float timeRate);

	/// <summary>
	/// �C�[�Y�C����ԁifloat�j
	/// </summary>
	/// <param name="start">�J�n�l</param>
	/// <param name="end">�I���l</param>
	/// <param name="timeRate">���Ԍo�߁i0�`1�j</param>
	/// <returns>��Ԃ��ꂽ�l</returns>
	static float EaseIn(const float start, const float end, const float timeRate);

	/// <summary>
	/// �C�[�Y�A�E�g��ԁifloat�j
	/// </summary>
	/// <param name="start">�J�n�l</param>
	/// <param name="end">�I���l</param>
	/// <param name="timeRate">���Ԍo�߁i0�`1�j</param>
	/// <returns>��Ԃ��ꂽ�l</returns>
	static float EaseOut(const float start, const float end, const float timeRate);

	/// <summary>
	/// �C�[�Y�C���A�E�g��ԁifloat�j
	/// </summary>
	/// <param name="start">�J�n�l</param>
	/// <param name="end">�I���l</param>
	/// <param name="timeRate">���Ԍo�߁i0�`1�j</param>
	/// <returns>��Ԃ��ꂽ�l</returns>
	static float EaseInOut(const float start, const float end, const float timeRate);


	float GetTimeRate() { return timeRate; }
};

