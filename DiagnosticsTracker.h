#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <string>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <deque>
#include <unordered_map>

#include "Box.h"
#include "Sphere.h"

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

	//Timer
	std::unordered_map<std::string, std::chrono::microseconds> functionDurations;
	std::unordered_map<std::string, time_point<high_resolution_clock>> startTimes;

	int* testArray;

	void InitCPUStats();

public:

	DiagnosticsTracker();

	void StartTimer(const std::string& functionName);
	void StopTimer(const std::string& functionName);

	void WalkTheHeap();

	void OutputMemoryAllocation();
	void OutputBoxMemoryAllocation();
	void OutputSphereMemoryAllocation();

	void TriggerBufferOverflow();

	void AllocateMemory(int size);
	void DeallocateMemory();

	std::string GetMemoryUsage();
	std::string GetCPUUsage();
	std::string GetFrameTime();
	std::string GetFPS();
	std::string GetThreadCount();
	std::string GetFunctionRunTime(const std::string& functionName);
	std::string GetTotalMemoryAllocated();
};

