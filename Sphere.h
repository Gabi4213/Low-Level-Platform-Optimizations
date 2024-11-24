#pragma once
#include "ColliderObject.h"
#include "MemoryPool.h"

class Sphere : public ColliderObject
{
private:
    static MemoryPool* sphereMemoryPool;

public:
    static void initalizeMemoryPool(size_t totalBytesAllocated);

    static void* operator new(size_t size);
    static void operator delete(void* poolMemory, size_t size);

    static MemoryPool* getMemoryPool();

    void drawMesh() { glutSolidSphere(0.5, 5, 5); }
}; 

