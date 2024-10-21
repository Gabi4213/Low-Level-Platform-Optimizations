#include "MemoryPool.h"


MemoryPool::MemoryPool(size_t totalSize)
{
	memoryPool = operator new(poolSize);
}

MemoryPool::~MemoryPool() 
{
	operator delete(memoryPool);
}

void* MemoryPool::AllocateMemory(size_t size)
{
	if (poolOffset + size > poolSize) 
	{
		//oh oh no good issue
		std::cout << "Not Enough memory in the memory pool" << std::endl;
		throw std::bad_alloc();
	}
	else 
	{
		void* memory = static_cast<char*>(memoryPool) + poolOffset;

		poolOffset + size;

		return memory;
	}
}

void MemoryPool::DeallocateMemory(void* poolMemory)
{

}
