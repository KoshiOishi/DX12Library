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

	//チャンクヘッダ
	struct Chunk
	{
		char id[4];	//チャンク毎のID
		int32_t size;
	};

	//RIFF ヘッダチャンク
	struct RiffHeader
	{
		Chunk chunk;	//"RIFF"
		char type[4];	//"WAVE"
	};

	//FMTチャンク
	struct FormatChunk
	{
		Chunk chunk;	//"fmt."
		WAVEFORMAT fmt;	//波形フォーマット
	};

	//読み込み後データ
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
	/// 音量をセットする
	/// </summary>
	/// <param name="key">ロードしたwavファイル二セットしたキー</param>
	/// <param name="volume">音量値(0～100)</param>
	static void SetVolume(string key, float volume);

	/// <summary>
	/// ピッチをセットする
	/// </summary>
	/// <param name="key">ロードしたwavファイル二セットしたキー</param>
	/// <param name="pitch">ピッチ(倍率指定)</param>
	static void SetPitch(string key, float pitch);

	static bool GetIsPlay(string key) { return datas[key].isPlay; }
	static bool GetIsPause(string key) { return datas[key].isPause; }
	static float GetPlayingPosition(string key);
	static float GetVolume(string key);
};


