#include "MemoryPool.h"
#include <stdexcept>
#include <iostream>

MemoryPool::MemoryPool(size_t totalSize, size_t blockSize)
{
    poolSize = totalSize;
    blocksSize = blockSize;
    memoryPool = operator new(poolSize);
}

MemoryPool::~MemoryPool()
{
    operator delete(memoryPool);
}

void* MemoryPool::AllocateMemory(size_t size)
{
    if (size > blocksSize)
    {
        throw std::runtime_error("Requested size is larger than block size");
    }

    if (freeList.empty() == false) 
    {
        void* memory = freeList.back();
        freeList.pop_back();
        return memory;
    }

    if (poolOffset + blocksSize > poolSize) 
    {
        throw std::runtime_error("Memory pool used up");
    }

    void* memory = static_cast<char*>(memoryPool) + poolOffset;
    poolOffset += blocksSize;
    return memory;
}

void MemoryPool::DeallocateMemory(void* poolMemory)
{
    if (poolMemory >= memoryPool && poolMemory < static_cast<char*>(memoryPool) + poolSize)
    {
        freeList.push_back(poolMemory);
    }
    else 
    {
        throw std::runtime_error("Pointer out of memory pool range");
    }
}

size_t MemoryPool::GetCurrentMemoryAllocated()
{
    return poolOffset - (freeList.size() * blocksSize);
}