#include "Box.h"
#include <iostream>


MemoryPool* Box::boxMemoryPool = nullptr;

void* Box::operator new(size_t size) 
{
    std::cout << "box operator new called with size: " << size << std::endl;

    if (!boxMemoryPool) 
    {
        boxMemoryPool = new MemoryPool(1024 * size, size);
    }
    return boxMemoryPool->AllocateMemory(size);
}

void Box::operator delete(void* poolMemory, size_t size) 
{
    std::cout << "Custom operator delete called for Box, size: " << size << std::endl;
    if (boxMemoryPool) 
    {
        boxMemoryPool->DeallocateMemory(poolMemory);
    }
}