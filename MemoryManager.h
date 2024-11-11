#pragma once
#include <iostream>
#include <mutex>

extern const unsigned int CHECK_VALUE;
extern size_t totalAllocatedMemory;

//Meyers Singleton since mutex issues
std::mutex& getAllocationMutex();

void* operator new(size_t size);
void operator delete(void* poolMemory);

struct MemoryAllocation
{
    size_t bytesAllocated;
    size_t bytesDeallocated;
    size_t bytes;
    size_t bytesSize;

    MemoryAllocation() : bytesAllocated(0), bytesDeallocated(0), bytes(0), bytesSize(0) {}
};

struct Header
{
    size_t size;
    Header* nextHeader;
    Header* previousHeader;
    unsigned int checkValue;
};

struct Footer
{
    unsigned int checkValue;
};

extern Header* firstAllocation;
extern Header* lastAllocation;

extern MemoryAllocation memoryAllocation;
