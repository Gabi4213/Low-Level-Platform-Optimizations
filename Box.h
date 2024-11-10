#pragma once

#include "ColliderObject.h"
#include "MemoryPool.h"

class Box : public ColliderObject
{
private:
	static MemoryPool* boxMemoryPool;
public:

	static void InitalizeMemoryPool(size_t totalBytesAllocated);

    static void* operator new(size_t size);
    static void operator delete(void* poolMemory, size_t size);

	static MemoryPool* GetMemoryPool();

	void drawMesh() { glutSolidCube(1.0); }
};

