#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include <stdexcept>

class MemoryPool
{
public:
    MemoryPool(size_t totalSize, size_t blockSize);
    ~MemoryPool();

    void* allocateMemory(size_t size);
    void deallocateMemory(void* poolMemory);

    size_t getCurrentMemoryAllocated();
private:
    void* memoryPool;
    size_t poolSize;
    size_t blocksSize;
    size_t poolOffset = 0;
    std::vector<void*> freeList; 
};
