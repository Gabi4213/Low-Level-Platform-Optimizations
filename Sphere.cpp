#include "Sphere.h"
#include <iostream>

MemoryPool* Sphere::sphereMemoryPool = nullptr;

void* Sphere::operator new(size_t size)
{
    std::cout << "sphere operator new called with size: " << size << std::endl;

    if (!sphereMemoryPool)
    {
        sphereMemoryPool = new MemoryPool(1024 * size, size);
    }
    return sphereMemoryPool->AllocateMemory(size);
}

void Sphere::operator delete(void* poolMemory, size_t size)
{
    std::cout << "sphere operator delete called with size: " << size << std::endl;
    if (sphereMemoryPool)
    {
        sphereMemoryPool->DeallocateMemory(poolMemory);
    }
}