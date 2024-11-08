#include "Sphere.h"
#include <iostream>

MemoryPool* Sphere::sphereMemoryPool = nullptr;

void Sphere::InitalizeMemoryPool(size_t totalBytesAllocated)
{
    sphereMemoryPool = new MemoryPool(totalBytesAllocated * sizeof(Sphere), sizeof(Sphere));
}

void* Sphere::operator new(size_t size)
{
    std::cout << "sphere operator new called with size: " << size << std::endl;

    if (sphereMemoryPool == nullptr)
    {
        std::cout << "memory pool not initalized. Call InitalizeMemoryPool()" << std::endl;
        //sphereMemoryPool = new MemoryPool(1024 * sphereBytesTotal, size); 
    }
    return sphereMemoryPool->AllocateMemory(size);
}

void Sphere::operator delete(void* poolMemory, size_t size)
{
    std::cout << "sphere operator delete called with size: " << size << std::endl;
    if (sphereMemoryPool != nullptr)
    {
        sphereMemoryPool->DeallocateMemory(poolMemory);
    }
}

MemoryPool* Sphere::GetMemoryPool()
{
    return sphereMemoryPool;
}