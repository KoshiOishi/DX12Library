#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <cstdint>
#include "XAudio2VoiceCallback.h"
#include <string>
#include <unordered_map>

#pragma comment(lib, "xaudio2.lib")

class Sound
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using string = std::string;

	//�`�����N�w�b�_
	struct Chunk
	{
		char id[4];	//�`�����N����ID
		int32_t size;
	};

	//RIFF �w�b�_�`�����N
	struct RiffHeader
	{
		Chunk chunk;	//"RIFF"
		char type[4];	//"WAVE"
	};

	//FMT�`�����N
	struct FormatChunk
	{
		Chunk chunk;	//"fmt."
		WAVEFORMAT fmt;	//�g�`�t�H�[�}�b�g
	};

	//�ǂݍ��݌�f�[�^
	struct SoundData
	{
		XAUDIO2_BUFFER buf;
		IXAudio2SourceVoice* pSourceVoice;
		XAudio2VoiceCallback voiceCallback;
		bool isPause;
		bool isPlay;
		bool isLoop;
	};


private:
	static ComPtr<IXAudio2> xAudio2;
	static IXAudio2MasteringVoice* masterVoice;
	static std::unordered_map<string, SoundData> datas;

public:
	static void DeleteSoundData();
	static void Initialize();
	static void Update();
	static void LoadWave(const char* filename, string key, bool isLoop = false);
	static void PlayWave(string key);
	static void StopWave(string key);
	static void PauseWave(string key);

	static void EndPlay(string key);

	/// <summary>
	/// ���ʂ��Z�b�g����
	/// </summary>
	/// <param name="key">���[�h����wav�t�@�C����Z�b�g�����L�[</param>
	/// <param name="volume">���ʒl(0�`100)</param>
	static void SetVolume(string key, float volume);

	/// <summary>
	/// �s�b�`���Z�b�g����
	/// </summary>
	/// <param name="key">���[�h����wav�t�@�C����Z�b�g�����L�[</param>
	/// <param name="pitch">�s�b�`(�{���w��)</param>
	static void SetPitch(string key, float pitch);

	static bool GetIsPlay(string key) { return datas[key].isPlay; }
	static bool GetIsPause(string key) { return datas[key].isPause; }
	static float GetPlayingPosition(string key);
	static float GetVolume(string key);
};


