#pragma once

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <string>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <deque>

using namespace std::chrono;

class DiagnosticsTracker
{
private:

	//Frame Time and FPS
	std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
	int frameCount = 0;
	float fps = 0.0f;

	//CPU
	ULONGLONG lastCPU = 0;
	ULONGLONG lastSysCPU = 0;
	ULONGLONG lastUserCPU = 0;
	int numProcessors = 0;

	std::chrono::high_resolution_clock::time_point lastUpdate;
	float cpuUsage = 0.0f;

	void InitCPUStats();

public:

	DiagnosticsTracker();

	std::string GetMemoryUsage();
	std::string GetCPUUsage();
	std::string GetFrameTime();
	std::string GetFPS();
	std::string GetThreadCount();
};

