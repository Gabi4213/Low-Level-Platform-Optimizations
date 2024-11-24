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
#include <thread>

#include "Box.h"
#include "Sphere.h"

using namespace std::chrono;

class DiagnosticsTracker
{
public:
	DiagnosticsTracker();

	void startTimer(std::string& functionName);
	void stopTimer(std::string& functionName);

	void walkTheHeap();

	void outputMemoryAllocation();
	void outputBoxMemoryAllocation();
	void outputSphereMemoryAllocation();

	void triggerBufferOverflow();
	void triggerMemoryCorruption();

	void allocateMemory(int size);
	void deallocateMemory();

	std::string getMemoryUsage();
	std::string getCPUUsage();
	std::string getFrameTime();
	std::string getFPS();
	std::string getFunctionRunTime(std::string& functionName);
	std::string getTotalMemoryAllocated();
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

	void initCPUStats();
};

