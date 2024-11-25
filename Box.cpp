#include "Box.h"
#include <iostream>

MemoryPool* Box::boxMemoryPool = nullptr;

void Box::initalizeMemoryPool(size_t totalBytesAllocated)
{
    //initializes the memory pool specific to box
    boxMemoryPool = new MemoryPool(totalBytesAllocated * sizeof(Box), sizeof(Box));
}

void* Box::operator new(size_t size)    
{
    //custom new operator for box
    std::cout << "box operator new called with size: " << size << std::endl;
   
    if (boxMemoryPool == nullptr) 
    {
        std::cout << "memory pool not initalized. Call InitalizeMemoryPool()" << std::endl;
    }
    return boxMemoryPool->allocateMemory(size);
}

void Box::operator delete(void* poolMemory, size_t size) 
{
    //custom delete operator for box
    std::cout << "box operator delete called with size: " << size << std::endl;
    if (boxMemoryPool != nullptr) 
    {
        boxMemoryPool->deallocateMemory(poolMemory);
    }
}

MemoryPool* Box::getMemoryPool()
{
    //retrieves the memory pool speficic to the box
    return boxMemoryPool;
}