#include "MemoryManager.h"
#include <iostream>

const unsigned int GUARD_VALUE = 0xDEADBEEF;
size_t totalAllocatedMemory = 0;

void* operator new (size_t size)
{
	std::cout << "new operator is being called" << std::endl;

	size_t totalSize = sizeof(Header) + sizeof(Footer) + size;
	char* poolMemory = (char*)malloc(totalSize);

	Header* headerPtr = (Header*)poolMemory;
	headerPtr->size = size;
	headerPtr->guardValue = GUARD_VALUE;

	Footer* footerPtr = (Footer*)(poolMemory + sizeof(Header) + size);
	footerPtr->guardValue = GUARD_VALUE;

	totalAllocatedMemory += size;

	void* pStartMemBlock = poolMemory + sizeof(Header);
	return pStartMemBlock;
}

void operator delete (void* poolMemory)
{
	std::cout << "delete operator is being called" << std::endl;

	Header* headerPtr = (Header*)((char*)poolMemory - sizeof(Header));
	Footer* footerPtr = (Footer*)((char*)poolMemory + headerPtr->size);

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
