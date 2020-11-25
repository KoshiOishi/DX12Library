#pragma once
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")

class FPSManager
{
private:
	static double FRAME_TIME;
	static LARGE_INTEGER start;
	static LARGE_INTEGER end;
	static LARGE_INTEGER freq;

	static double fps;

public:
	static void Initialize(double fps = 60.0);
	static void AdjustFPS();
	static double GetFPS() { return fps; }
};

