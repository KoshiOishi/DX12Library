#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <cstdint>
#include "XAudio2VoiceCallback.h"

#pragma comment(lib, "xaudio2.lib")

class Sound
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

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

private:
	static ComPtr<IXAudio2> xAudio2;
	static IXAudio2MasteringVoice* masterVoice;
	static XAudio2VoiceCallback voiceCallback;


public:
	static void Initialize();
	static void PlayWave(const char* filename);
};

