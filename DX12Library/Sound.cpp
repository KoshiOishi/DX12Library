#include "Sound.h"
#include <fstream>
#include <cassert>

Sound::ComPtr<IXAudio2> Sound::xAudio2;
IXAudio2MasteringVoice* Sound::masterVoice;
std::unordered_map<std::string, Sound::SoundData> Sound::datas;

void Sound::DeleteSoundData()
{

	for (auto itr = datas.begin(); itr != datas.end(); itr++)
	{
		if (itr->second.pSourceVoice)
		{
			itr->second.pSourceVoice->Stop(0);
			itr->second.pSourceVoice->DestroyVoice();
			itr->second.pSourceVoice = NULL;
		}
	}

	if (masterVoice != nullptr)
	{
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}

	CoUninitialize();
}

void Sound::Initialize()
{
	HRESULT result;

	//COMの初期化
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
		{
			assert(0);
			return;
		}
	}

	//XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	//マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
}

void Sound::Update()
{
	for (auto itr = datas.begin(); itr != datas.end(); itr++)
	{
		if (GetPlayingPosition(itr->first) == 0 && itr->second.isPlay == true)
		{
			itr->second.isPlay = false;
			itr->second.isPause = false;
		}
	}
}

void Sound::LoadWave(const char * filename, string key, bool isLoop)
{
	HRESULT result;

	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	if (file.fail()) {
		assert(0);
	}


	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	//Formatチャンクの読み込み
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	//Dataチャンクの読み込み
	Chunk data;
	file.read((char*)&data, sizeof(data));

	//Dataチャンクのデータ部(刷毛データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Waveファイルを閉じる
	file.close();

	//サウンド再生
	WAVEFORMATEX wfex{};

	//波形フォーマットの設定
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f, &datas[key].voiceCallback);
	if FAILED(result) {
		delete[] pBuffer;
		return;
	}

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)pBuffer;
	buf.pContext = pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;
	if (isLoop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	result = pSourceVoice->SubmitSourceBuffer(&buf);

	datas[key].buf = buf;
	datas[key].pSourceVoice = pSourceVoice;

}

void Sound::PlayWave(string key)
{
	HRESULT result = S_FALSE;
	if (datas[key].isPlay) StopWave(key);

	//波形データの再生
	if (datas[key].isPause == false) {
		datas[key].pSourceVoice->FlushSourceBuffers();
		datas[key].pSourceVoice->SubmitSourceBuffer(&datas[key].buf);
	}
	result = datas[key].pSourceVoice->Start();
	datas[key].isPause = false;

	datas[key].isPlay = true;
}

void Sound::StopWave(string key)
{
	HRESULT result = S_FALSE;
	result = datas[key].pSourceVoice->Stop();
	datas[key].isPlay = false;
	if (datas[key].isLoop)
		datas[key].pSourceVoice->ExitLoop();
}

void Sound::PauseWave(string key)
{
	HRESULT result = S_FALSE;
	result = datas[key].pSourceVoice->Stop();
	datas[key].isPause = true;
	datas[key].isPlay = false;
	GetPlayingPosition(key);
}

void Sound::SetVolume(string key, float volume)
{
	HRESULT result = S_FALSE;
	float f = volume / 100.0f;
	result = datas[key].pSourceVoice->SetVolume(f);
}

void Sound::SetPitch(string key, float pitch)
{
	HRESULT result = S_FALSE;
	result = datas[key].pSourceVoice->SetFrequencyRatio(pitch);
}

float Sound::GetPlayingPosition(string key)
{
	XAUDIO2_VOICE_STATE* xvs = new XAUDIO2_VOICE_STATE;
	datas[key].pSourceVoice->GetState(xvs);
	return (float)xvs->SamplesPlayed;
}

float Sound::GetVolume(string key)
{
	HRESULT result = S_FALSE;
	float* f = new float;
	datas[key].pSourceVoice->GetVolume(f);
	return *f * 100.0f;
}
