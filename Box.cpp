#include "Box.h"
#include <iostream>

MemoryPool* Box::boxMemoryPool = nullptr;

void Box::InitalizeMemoryPool(size_t totalBytesAllocated)
{
    boxMemoryPool = new MemoryPool(totalBytesAllocated * sizeof(Box), sizeof(Box));
}

void* Box::operator new(size_t size)    
{
    std::cout << "box operator new called with size: " << size << std::endl;
   
    if (boxMemoryPool == nullptr) 
    {
        std::cout << "memory pool not initalized. Call InitalizeMemoryPool()" << std::endl;
       // boxMemoryPool = new MemoryPool(1024 * size, size);
    }
    return boxMemoryPool->AllocateMemory(size);
}

void Box::operator delete(void* poolMemory, size_t size) 
{
    std::cout << "box operator delete called with size: " << size << std::endl;
    if (boxMemoryPool != nullptr) 
    {
        boxMemoryPool->DeallocateMemory(poolMemory);
    }
}