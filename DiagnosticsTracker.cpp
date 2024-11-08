#include "DiagnosticsTracker.h"
#include "MemoryManager.h"

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

std::string DiagnosticsTracker::GetTotalMemoryAllocated()
{
    return std::to_string(totalAllocatedMemory);
}


void DiagnosticsTracker::StartTimer(const std::string& functionName)
{
    startTimes[functionName] = high_resolution_clock::now();
}

void DiagnosticsTracker::StopTimer(const std::string& functionName)
{
    auto stopTime = high_resolution_clock::now();

    microseconds duration = duration_cast<microseconds>(stopTime - startTimes[functionName]);

    functionDurations[functionName] = duration;
}

std::string DiagnosticsTracker::GetFunctionRunTime(const std::string& functionName)
{
    return std::to_string(functionDurations[functionName].count());
}

void DiagnosticsTracker::WalkTheHeap()
{
    Header* currentHeader = firstAllocation;

    while (currentHeader != nullptr)
    {
        Footer* currentFooter = (Footer*)((char*)currentHeader + sizeof(Header) + currentHeader->size);

        std::cout << "Current Header at: " << currentHeader << "\n\Check Value: " << currentHeader->checkValue << "\n\tPrevious Header: " << currentHeader->previousHeader << "\n\tNext Header: " << currentHeader->nextHeader;

        if (currentHeader->checkValue == currentFooter->checkValue)
        {
            std::cout << "\n\tNOT Corrupt" << std::endl;
        }
        else
        {
            std::cout << "\n\tCorrupt" << std::endl;
        }

        std::cout << "\nCurrent Footer at: " << currentFooter << "\n\Check Value: " << currentFooter->checkValue << "\n\tSize: " << currentHeader->size << "\n\n";

        currentHeader = currentHeader->nextHeader;
    }
}

void DiagnosticsTracker::OutputMemoryAllocation()
{
    std::cout << "\nBytes Allocated: " << memoryAllocation.bytesAllocated << "\nBytes Deallocated: " << memoryAllocation.bytesDeallocated << "\nBytes: " << memoryAllocation.bytes << "\n\n";
}

void DiagnosticsTracker::OutputBoxMemoryAllocation()
{    
    std::cout << "Current Boxes Memory Allocated: " << Box::GetMemoryPool()->GetCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::OutputSphereMemoryAllocation()
{
    std::cout << "Current Spheres Memory Allocated: " << Sphere::GetMemoryPool()->GetCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::TriggerBufferOverflow()
{
    int* x = new int[10];

    for (int i = 0; i < 15; i++) 
    {
        x[i] = i;
    }

    delete[] x;
}

void DiagnosticsTracker::AllocateMemory(int size)
{
    testArray = new int[size];
    std::cout << "\nBytes Allocated 4 * " << size << "= " << memoryAllocation.bytesSize << std::endl;
}

void DiagnosticsTracker::DeallocateMemory()
{
    delete[] testArray;
    std::cout << "\nBytes Deallocated, Current Size: " << memoryAllocation.bytesSize << std::endl;
}