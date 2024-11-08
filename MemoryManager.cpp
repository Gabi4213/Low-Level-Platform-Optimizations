#include "MemoryManager.h"
#include <iostream>
#include <mutex>

const unsigned int CHECK_VALUE = 0xDEADBEEF;
size_t totalAllocatedMemory = 0;

Header* firstAllocation{ nullptr };
Header* lastAllocation{ nullptr };

MemoryAllocation memoryAllocation;

std::mutex mutex;

void* operator new (size_t size)
{
	//std::cout << "new operator is being called" << std::endl;

	std::lock_guard<std::mutex> lock(mutex);

	size_t totalSize = sizeof(Header) + sizeof(Footer) + size;

	memoryAllocation.bytesAllocated += totalSize;
	memoryAllocation.bytes += totalSize;
	memoryAllocation.bytesSize = size;

	char* poolMemory = (char*)malloc(totalSize);

	if (poolMemory == nullptr)
	{
		throw std::bad_alloc();
	}

	Header* headerPtr = (Header*)poolMemory;
	headerPtr->size = size;
	headerPtr->checkValue = CHECK_VALUE;
	headerPtr->previousHeader = lastAllocation;
	headerPtr->nextHeader = nullptr;

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
	footerPtr->checkValue = CHECK_VALUE;

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

	std::lock_guard<std::mutex> lock(mutex);

	//std::cout << "delete operator is being called" << std::endl;

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

	if (headerPtr->checkValue != CHECK_VALUE)
	{
		std::cout << "header check value doesnt match! Buffer Overflow!" << std::endl;
		return;
	}

	if (footerPtr->checkValue != CHECK_VALUE)
	{
		std::cout << "footer check value doesnt match! Buffer Overflow!" << std::endl;
		return;
	}

	totalAllocatedMemory -= headerPtr->size;

	memoryAllocation.bytesSize = 0;

	memoryAllocation.bytesDeallocated += sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);
	memoryAllocation.bytes -= sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);

	free(headerPtr);
}