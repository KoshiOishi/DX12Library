#include "FPSManager.h"

double FPSManager::FRAME_TIME = 0.0f;
LARGE_INTEGER FPSManager::start;
LARGE_INTEGER FPSManager::end;
LARGE_INTEGER FPSManager::freq;

double FPSManager::fps;

void FPSManager::Initialize(double fps)
{
	FRAME_TIME = 1.0f / fps;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
}

void FPSManager::AdjustFPS()
{
	QueryPerformanceCounter(&end);
	double d = end.QuadPart - start.QuadPart;
	double frameTime = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(freq.QuadPart);
	
	if (frameTime < FRAME_TIME)
	{
		DWORD sleepTime = static_cast<DWORD>((FRAME_TIME - frameTime) * 1000);
 		timeBeginPeriod(1);
		Sleep(sleepTime);
		timeEndPeriod(1);
	}

	fps = 1.0f / frameTime;

}
