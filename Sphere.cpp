#include "Sphere.h"
#include <iostream>

MemoryPool* Sphere::sphereMemoryPool = nullptr;

void Sphere::initalizeMemoryPool(size_t totalBytesAllocated)
{
    //initializes the memory pool specific to sphere
    sphereMemoryPool = new MemoryPool(totalBytesAllocated * sizeof(Sphere), sizeof(Sphere));
}

void* Sphere::operator new(size_t size)
{
    //custom new operator for sphere
    std::cout << "sphere operator new called with size: " << size << " Bytes" << std::endl;

    if (sphereMemoryPool == nullptr)
    {
        std::cout << "memory pool not initalized. Call InitalizeMemoryPool()" << std::endl;
    }
    return sphereMemoryPool->allocateMemory(size);
}

void Sphere::operator delete(void* poolMemory, size_t size)
{
    //custom delete operator for sphere
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
    //retrieves the memory pool speficic to the sphere
    return sphereMemoryPool;
}