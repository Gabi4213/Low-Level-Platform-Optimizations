#include "MemoryPool.h"
#include <stdexcept>
#include <iostream>

MemoryPool::MemoryPool(size_t totalSize, size_t blockSize)
{
    //initialize variables and create new memory pool
    poolSize = totalSize;
    blocksSize = blockSize;
    memoryPool = operator new(poolSize);
}

MemoryPool::~MemoryPool()
{
    //delete memory pool
    operator delete(memoryPool);
}

void* MemoryPool::allocateMemory(size_t size)
{
    //error check
    if (size > blocksSize)
    {
        throw std::runtime_error("Requested size is larger than block size!");
    }

    //if the list is NOT empty. Then just reuse a previously deallocated block
    if (freeList.empty() == false) 
    {
        void* memory = freeList.back();
        freeList.pop_back();
        return memory;
    }

    //check if there is enough space
    if (poolOffset + blocksSize > poolSize) 
    {
        throw std::runtime_error("Memory pool used up!");
    }

    //allocate a new memory block
    void* memory = static_cast<char*>(memoryPool) + poolOffset;
    poolOffset += blocksSize;
    return memory;
}

void MemoryPool::deallocateMemory(void* poolMemory)
{
    //check if the pointer is within the range of the memory pool. if so add it to free list
    if (poolMemory >= memoryPool && poolMemory < static_cast<char*>(memoryPool) + poolSize)
    {
        freeList.push_back(poolMemory);
    }
    else 
    {
        throw std::runtime_error("Pointer out of memory pool range!");
    }
}

size_t MemoryPool::getCurrentMemoryAllocated()
{
    //retrieves the total memory allocated currently 
    return poolOffset - (freeList.size() * blocksSize);
}