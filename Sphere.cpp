#include "Sphere.h"
#include <iostream>

MemoryPool* Sphere::sphereMemoryPool = nullptr;

void Sphere::initalizeMemoryPool(size_t totalBytesAllocated)
{
    sphereMemoryPool = new MemoryPool(totalBytesAllocated * sizeof(Sphere), sizeof(Sphere));
}

void* Sphere::operator new(size_t size)
{
    std::cout << "sphere operator new called with size: " << size << " Bytes" << std::endl;

    if (sphereMemoryPool == nullptr)
    {
        std::cout << "memory pool not initalized. Call InitalizeMemoryPool()" << std::endl;
    }
    return sphereMemoryPool->allocateMemory(size);
}

void Sphere::operator delete(void* poolMemory, size_t size)
{
    std::cout << "sphere operator delete called with size: " << size << " Bytes" << std::endl;
    if (sphereMemoryPool != nullptr)
    {
        sphereMemoryPool->deallocateMemory(poolMemory);
    }
    else 
    {
        std::cout << "memory already deallocated!" << std::endl;
    }
}

MemoryPool* Sphere::getMemoryPool()
{
    return sphereMemoryPool;
}