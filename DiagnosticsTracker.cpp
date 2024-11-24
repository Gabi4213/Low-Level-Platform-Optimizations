#include "DiagnosticsTracker.h"
#include "MemoryManager.h"

DiagnosticsTracker::DiagnosticsTracker()
{
    initCPUStats();
    lastUpdate = std::chrono::high_resolution_clock::now();
}

std::string DiagnosticsTracker::getMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemoryCounters, sizeof(processMemoryCounters));
    size_t memoryUsage = processMemoryCounters.PrivateUsage;
    return std::to_string(memoryUsage);
}

void DiagnosticsTracker::initCPUStats()
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

std::string DiagnosticsTracker::getCPUUsage()
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

std::string DiagnosticsTracker::getFrameTime()
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

std::string DiagnosticsTracker::getFPS()
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

std::string DiagnosticsTracker::getTotalMemoryAllocated()
{
    return std::to_string(totalAllocatedMemory);
}


void DiagnosticsTracker::startTimer(std::string& functionName)
{
    startTimes[functionName] = high_resolution_clock::now();
}

void DiagnosticsTracker::stopTimer(std::string& functionName)
{
    auto stopTime = high_resolution_clock::now();

    microseconds duration = duration_cast<microseconds>(stopTime - startTimes[functionName]);

    functionDurations[functionName] = duration;
}

std::string DiagnosticsTracker::getFunctionRunTime(std::string& functionName)
{
    return std::to_string(functionDurations[functionName].count());
}

void DiagnosticsTracker::walkTheHeap()
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
void DiagnosticsTracker::triggerMemoryCorruption()
{
    if (firstAllocation != nullptr)
    {
        // invert the check vaslue. I do this since I cant actually set the check value to something as its a const
        firstAllocation->checkValue = ~firstAllocation->checkValue; 
        walkTheHeap();
    }
    else
    {
        std::cout << "firstAllocation null nothing to corrupt." << std::endl;
    }
}

void DiagnosticsTracker::outputMemoryAllocation()
{
    std::cout << "\nTotal Bytes Allocated: " << memoryAllocation.bytesAllocated << "\nTotal Bytes Deallocated: " << memoryAllocation.bytesDeallocated << "\nTotal Current Bytes: " << memoryAllocation.bytes << "\n\n";
}

void DiagnosticsTracker::outputBoxMemoryAllocation()
{    
    std::cout << "Current Boxes Memory Allocated: " << Box::getMemoryPool()->getCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::outputSphereMemoryAllocation()
{
    std::cout << "Current Spheres Memory Allocated: " << Sphere::getMemoryPool()->getCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::triggerBufferOverflow()
{
    int* x = new int[10];

    for (int i = 0; i < 15; i++) 
    {
        x[i] = i;
    }

    delete[] x;
    x = nullptr;
}

void DiagnosticsTracker::allocateMemory(int size)
{
    testArray = new int[size];
    std::cout << "\nBytes Allocated 4 * " << size << "= " << memoryAllocation.bytesSize << std::endl;
}

void DiagnosticsTracker::deallocateMemory()
{
    delete[] testArray;
    testArray = nullptr;
    std::cout << "\nBytes Deallocated, Current Size: " << memoryAllocation.bytesSize << std::endl;
}