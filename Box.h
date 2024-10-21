#pragma once

#include "ColliderObject.h"
#include "MemoryPool.h"

class Box : public ColliderObject
{
private:


public:

	Box();
	MemoryPool* boxMemoryPool;
	static void* operator new(size_t size);
	void drawMesh() { glutSolidCube(1.0); }
};

