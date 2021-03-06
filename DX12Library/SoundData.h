#pragma once
#include <xaudio2.h>
#include "XAudio2VoiceCallback.h"
#pragma comment(lib, "xaudio2.lib")

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
