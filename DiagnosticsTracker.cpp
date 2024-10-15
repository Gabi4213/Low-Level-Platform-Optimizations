#include "DiagnosticsTracker.h"

DiagnosticsTracker::DiagnosticsTracker() 
{
    InitCPUStats();
    lastUpdate = std::chrono::high_resolution_clock::now();
}

std::string DiagnosticsTracker::GetMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemoryCounters, sizeof(processMemoryCounters));
    size_t memoryUsage = processMemoryCounters.PrivateUsage;
    return std::to_string(memoryUsage);
}

void DiagnosticsTracker::InitCPUStats()
{
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    HANDLE hProcess = GetCurrentProcess();
    GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

std::string DiagnosticsTracker::GetCPUUsage()
{
    FILETIME ftime, fsys, fuser;
    ULONGLONG now, sys, user;

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration<float>(currentTime - lastUpdate).count();

    if (elapsedTime >= 1.0f)
    {
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));

        HANDLE hProcess = GetCurrentProcess();
        GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));

        ULONGLONG totalCPU = (sys - lastSysCPU) + (user - lastUserCPU);
        ULONGLONG totalTime = now - lastCPU;

        if (totalTime > 0)
        {
            cpuUsage = (totalCPU * 100.0f) / (totalTime * numProcessors);
        }

        lastCPU = now;
        lastSysCPU = sys;
        lastUserCPU = user;
        lastUpdate = currentTime;
    }

    return std::to_string(cpuUsage);
}

std::string DiagnosticsTracker::GetFrameTime()
{
    float frameTime;

    if (fps > 0)
    {
        frameTime = 1000.0f / fps;
    }
    else
    {
        frameTime = 0.0f;
    }

    return std::to_string(frameTime);
}

std::string DiagnosticsTracker::GetFPS()
{
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration<float>(currentTime - lastTime).count();

    if (elapsedTime > 1.0f)
    {
        fps = frameCount / elapsedTime;
        frameCount = 0;
        lastTime = currentTime;
    }

    return std::to_string(fps);
}

std::string DiagnosticsTracker::GetThreadCount() 
{
    HANDLE hProcess = GetCurrentProcess();
    DWORD threadCount = 0; 

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    return std::to_string(threadCount);
}