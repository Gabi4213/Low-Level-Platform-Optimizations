#include "MemoryManager.h"
#include <iostream>

void* operator new (size_t size)
{
	std::cout << "new operator is being called" << std::endl;

	char* poolMemory = (char*)malloc(size);
	void* pStartMemBlock = poolMemory;
	return pStartMemBlock;
}

void operator delete (void* poolMemory)
{
	std::cout << "delete operator is being called" << std::endl;

	free(poolMemory);
}
