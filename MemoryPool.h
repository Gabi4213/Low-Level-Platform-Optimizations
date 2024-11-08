#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include <stdexcept>

class MemoryPool
{
private:
    void* memoryPool;
    size_t poolSize;
    size_t blocksSize;
    size_t poolOffset = 0;
    std::vector<void*> freeList; 

public:

    MemoryPool(size_t totalSize, size_t blockSize);
    ~MemoryPool();

    void* AllocateMemory(size_t size);
    void DeallocateMemory(void* poolMemory);

    size_t GetCurrentMemoryAllocated();
};
