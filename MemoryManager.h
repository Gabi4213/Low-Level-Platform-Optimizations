#pragma once
#include <iostream>
#include <mutex>

extern const unsigned int CHECK_VALUE;
extern size_t totalAllocatedMemory;

void* operator new (size_t size);
void operator delete (void* poolMemory);

struct MemoryAllocation
{
	size_t bytesAllocated;
	size_t bytesDeallocated;
	size_t bytes;

	size_t bytesSize;

	MemoryAllocation()
	{
		this->bytesAllocated = 0;
		this->bytesDeallocated = 0;
		this->bytes = 0;
	}
};

struct Header
{
	size_t size;
	Header* nextHeader;
	Header* previousHeader;
	unsigned int checkValue;
};

struct Footer
{
	unsigned int checkValue;
};

extern Header* firstAllocation;
extern Header* lastAllocation;

extern MemoryAllocation memoryAllocation;