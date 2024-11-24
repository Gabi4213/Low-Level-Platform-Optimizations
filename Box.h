#pragma once

#include "ColliderObject.h"
#include "MemoryPool.h"

class Box : public ColliderObject
{
public:
	static void initalizeMemoryPool(size_t totalBytesAllocated);

    static void* operator new(size_t size);
    static void operator delete(void* poolMemory, size_t size);

	static MemoryPool* getMemoryPool();

	void drawMesh() { glutSolidCube(1.0); }
private:
	static MemoryPool* boxMemoryPool;
};

