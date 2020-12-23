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

	//COM�̏�����
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
		{
			assert(0);
			return;
		}
	}

	//XAudio�G���W���̃C���X�^���X�𐶐�
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	//�}�X�^�[�{�C�X�𐶐�
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

	//�t�@�C�����̓X�g���[���̃C���X�^���X
	std::ifstream file;
	//.wav�t�@�C�����o�C�i�����[�h�ŊJ��
	file.open(filename, std::ios_base::binary);
	//�t�@�C���I�[�v�����s�����o����
	if (file.fail()) {
		assert(0);
	}


	//RIFF�w�b�_�[�̓ǂݍ���
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//�t�@�C����RIFF���`�F�b�N
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	//Format�`�����N�̓ǂݍ���
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	//Data�`�����N�̓ǂݍ���
	Chunk data;
	file.read((char*)&data, sizeof(data));

	//Data�`�����N�̃f�[�^��(���уf�[�^)�̓ǂݍ���
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Wave�t�@�C�������
	file.close();

	//�T�E���h�Đ�
	WAVEFORMATEX wfex{};

	//�g�`�t�H�[�}�b�g�̐ݒ�
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

	//�g�`�t�H�[�}�b�g������SourceVoice�̐���
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f, &datas[key].voiceCallback);
	if FAILED(result) {
		delete[] pBuffer;
		return;
	}

	//�Đ�����g�`�f�[�^�̐ݒ�
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

	//�g�`�f�[�^�̍Đ�
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
