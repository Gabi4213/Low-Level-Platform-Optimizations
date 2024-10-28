#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include <stdexcept>

class MemoryPool
{
public:

    MemoryPool(size_t totalSize, size_t blocksSize);
    ~MemoryPool();

    void* AllocateMemory(size_t size);
    void DeallocateMemory(void* poolMemory);

private:

    void* memoryPool;
    size_t poolSize;
    size_t blocksSize;
    size_t poolOffset = 0;
    std::vector<void*> freeList; 
};
