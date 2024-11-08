#pragma once
#include "ColliderObject.h"
#include "MemoryPool.h"
#include "globals.h"

class Sphere : public ColliderObject
{
private:
    static MemoryPool* sphereMemoryPool;

public:

    static void InitalizeMemoryPool(size_t totalBytesAllocated);

    static void* operator new(size_t size);
    static void operator delete(void* poolMemory, size_t size);

    static MemoryPool* GetMemoryPool();

    void drawMesh() { glutSolidSphere(0.5, 5, 5); }
}; 

