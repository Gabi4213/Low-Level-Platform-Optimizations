#include "MemoryManager.h"
#include <iostream>

const unsigned int GUARD_VALUE = 0xDEADBEEF;
size_t totalAllocatedMemory = 0;

Header* firstAllocation{ nullptr };
Header* lastAllocation{ nullptr };

void* operator new (size_t size)
{
	std::cout << "new operator is being called" << std::endl;

	size_t totalSize = sizeof(Header) + sizeof(Footer) + size;
	char* poolMemory = (char*)malloc(totalSize);

	if (poolMemory == nullptr)
	{
		throw std::bad_alloc();
	}

	Header* headerPtr = (Header*)poolMemory;
	headerPtr->size = size;
	headerPtr->guardValue = GUARD_VALUE;
	headerPtr->nextHeader = nullptr;
	headerPtr->previousHeader = lastAllocation;

	if (lastAllocation != nullptr)
	{
		lastAllocation->nextHeader = headerPtr;
	}

	lastAllocation = headerPtr;

	if (firstAllocation == nullptr)
	{
		firstAllocation = headerPtr;
	}

	Footer* footerPtr = (Footer*)(poolMemory + sizeof(Header) + size);
	footerPtr->guardValue = GUARD_VALUE;

	totalAllocatedMemory += size;

	void* pStartMemBlock = poolMemory + sizeof(Header);
	return pStartMemBlock;
}

void operator delete (void* poolMemory)
{
	if (poolMemory == nullptr)
	{
		std::cout << "poolMemory is nullptr. Nothing to delete" << std::endl;
		return;
	}

	std::cout << "delete operator is being called" << std::endl;

	Header* headerPtr = (Header*)((char*)poolMemory - sizeof(Header));
	Footer* footerPtr = (Footer*)((char*)poolMemory + headerPtr->size);

	if (headerPtr == firstAllocation) 
	{
		firstAllocation = headerPtr->nextHeader;
	}
	else 
	{
		headerPtr->previousHeader->nextHeader = headerPtr->nextHeader;
	}

	if (headerPtr == lastAllocation)
	{
		lastAllocation = headerPtr->previousHeader;
	}
	else
	{
		headerPtr->nextHeader->previousHeader = headerPtr->previousHeader;
	}

	if (headerPtr->guardValue != GUARD_VALUE)
	{
		std::cout << "header guard value doesnt match!" << std::endl;
		return;
	}

	if (footerPtr->guardValue != GUARD_VALUE)
	{
		std::cout << "footer guard value doesnt match!" << std::endl;
		return;
	}

	totalAllocatedMemory -= headerPtr->size;

	free(headerPtr);
}