#include "Box.h"

Box::Box() 
{
    boxMemoryPool = new MemoryPool(1024);
}


void* Box::operator new(size_t size)
{
    return boxMemoryPool
}