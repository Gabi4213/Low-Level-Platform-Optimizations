#include "DiagnosticsTracker.h"
#include "MemoryManager.h"

DiagnosticsTracker::DiagnosticsTracker()
{
    //call the CPU stuff on initizlizations becuase it uses chrono clock and we need to get 
    //reference to the time now asap
    initCPUStats();
    lastUpdate = std::chrono::high_resolution_clock::now();
}

std::string DiagnosticsTracker::getMemoryUsage()
{
    //accesses the memory usage. I am using private because its value is what i see under diagnostic tools
    //specifically its the process memory. I tried differnt options and it seemed like it was always not acurate
    //https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
    PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemoryCounters, sizeof(processMemoryCounters));
    size_t memoryUsage = processMemoryCounters.PrivateUsage;
    return std::to_string(memoryUsage);
}

void DiagnosticsTracker::initCPUStats()
{
    //intialize CPU usage stuff
    //https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
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
    // get cpu usage
    //https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
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

        if (totalTime > 0.0f)
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
    //get the frame time. we can use fps for this
    float frameTime;

    if (fps > 0.0f)
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
    //get the fps. pretty simple just used chrono
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration<float>(currentTime - lastTime).count();

    if (elapsedTime > 1.0f)
    {
        fps = frameCount / elapsedTime;
        frameCount = 0.0f;
        lastTime = currentTime;
    }

    return std::to_string(fps);
}

std::string DiagnosticsTracker::getTotalMemoryAllocated()
{
    //return the total allocated memory
    return std::to_string(totalAllocatedMemory);
}


void DiagnosticsTracker::startTimer(std::string& functionName)
{
    //start the timer. do be able to have multiple timers i made it a unordered map
    // and i add the function name to define them
    startTimes[functionName] = high_resolution_clock::now();
}

void DiagnosticsTracker::stopTimer(std::string& functionName)
{
    //stops the timer and again uses the unordered map to update them individually
    auto stopTime = high_resolution_clock::now();

    microseconds duration = duration_cast<microseconds>(stopTime - startTimes[functionName]);

    functionDurations[functionName] = duration;
}

std::string DiagnosticsTracker::getFunctionRunTime(std::string& functionName)
{
    //returns the function run time. using the unordered map like before
    return std::to_string(functionDurations[functionName].count());
}

void DiagnosticsTracker::walkTheHeap()
{
    //this i the walking the heap function.
    //start by setting the header to the first allocation
    Header* currentHeader = firstAllocation;

    //go through it until its no null
    while (currentHeader != nullptr)
    {
        //get the current footer
        Footer* currentFooter = (Footer*)((char*)currentHeader + sizeof(Header) + currentHeader->size);

        //output to console
        std::cout << "Current Header at: " << currentHeader << "\n\Check Value: " << currentHeader->checkValue << "\n\tPrevious Header: " << currentHeader->previousHeader << "\n\tNext Header: " << currentHeader->nextHeader;

        //check their check values. aka is it corrupt
        if (currentHeader->checkValue == currentFooter->checkValue)
        {
            std::cout << "\n\tNOT Corrupt" << std::endl;
        }
        else
        {
            std::cout << "\n\tCorrupt" << std::endl;
        }
        std::cout << "\nCurrent Footer at: " << currentFooter << "\n\Check Value: " << currentFooter->checkValue << "\n\tSize: " << currentHeader->size << "\n\n";

        //go to the next one
        currentHeader = currentHeader->nextHeader;
    }
}
void DiagnosticsTracker::triggerMemoryCorruption()
{
    //error check
    if (firstAllocation != nullptr)
    {
        // invert the check vaslue. I do this since I cant actually set the check value to something as its a const
        //this way it still wont be the same and will cause corruption
        firstAllocation->checkValue = ~firstAllocation->checkValue; 

        //walk the heap immidietly to display this
        walkTheHeap();
    }
    else
    {
        std::cout << "firstAllocation null nothing to corrupt." << std::endl;
    }
}

void DiagnosticsTracker::outputMemoryAllocation()
{
    //outputs the memory allocation data
    std::cout << "\nTotal Bytes Allocated: " << memoryAllocation.bytesAllocated << "\nTotal Bytes Deallocated: " << memoryAllocation.bytesDeallocated << "\nTotal Current Bytes: " << memoryAllocation.bytes << "\n\n";
}

void DiagnosticsTracker::outputBoxMemoryAllocation()
{    
    //outputs the memory allocation data specific to box
    std::cout << "Current Boxes Memory Allocated: " << Box::getMemoryPool()->getCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::outputSphereMemoryAllocation()
{
    //outputs the memory allocation data specific to sphere
    std::cout << "Current Spheres Memory Allocated: " << Sphere::getMemoryPool()->getCurrentMemoryAllocated() << " Bytes\n";
}

void DiagnosticsTracker::triggerBufferOverflow()
{
    //cause buffer overflow. This is pretty simple just try to allocate data beyond the arrays size
    int* x = new int[10];

    for (int i = 0; i < 15; i++) 
    {
        x[i] = i;
    }

    //clean it up
    delete[] x;
    x = nullptr;
}

void DiagnosticsTracker::allocateMemory(int size)
{
    //this function is used to test that allocations work. So just allocated a certain amount of memory
    testArray = new int[size];
    std::cout << "\nBytes Allocated 4 * " << size << "= " << memoryAllocation.bytesSize << std::endl;
}

void DiagnosticsTracker::deallocateMemory()
{
    //this function is used to test that deallocation work. So just deallocates a certain amount of memory
    delete[] testArray;
    testArray = nullptr;
    std::cout << "\nBytes Deallocated, Current Size: " << memoryAllocation.bytesSize << std::endl;
}