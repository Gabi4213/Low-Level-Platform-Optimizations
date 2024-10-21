#pragma once
#include<array>
#include <iostream>
#include <cstddef>
#include <stdexcept>

class MemoryPool
{
private:
	~MemoryPool();

	void* memoryPool;
	size_t poolSize;
	size_t poolOffset;

public:
	MemoryPool(size_t totalSize);
	void* AllocateMemory(size_t size);
	void DeallocateMemory(void* poolMemory);
};

