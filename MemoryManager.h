#pragma once

extern const unsigned int GUARD_VALUE;
extern size_t totalAllocatedMemory;

void* operator new (size_t size);
void operator delete (void* poolMemory);

struct Header 
{
	size_t size;
	Header* nextHeader;
	Header* previousHeader;
	unsigned int guardValue;
};

struct Footer 
{
	unsigned int guardValue;
};

extern Header* firstAllocation;
extern Header* lastAllocation;