#include "MemoryManager.h"
#include <cstdlib>

const unsigned int CHECK_VALUE = 0xDEADBEEF;
size_t totalAllocatedMemory = 0;

// Accessor function for the allocation mutex
// i do this becuase an issue i enouctered is that somtimes the mutex is not initialized before being used. 
// this was to do with the way visual studio would compile. Sometimes it did initialize othertimes it didn't.
//this makes sure that at whatever point i retrieve it, it is actually initialized.
std::mutex& getAllocationMutex()
{
    static std::mutex allocationMutex;
    return allocationMutex;
}

Header* firstAllocation{ nullptr };
Header* lastAllocation{ nullptr };

MemoryAllocation memoryAllocation;

void* operator new(size_t size)
{
    //lock mutex for safety
    std::lock_guard<std::mutex> lock(getAllocationMutex());

    //initialize data and add to struct totals
    size_t totalSize = sizeof(Header) + sizeof(Footer) + size;
    memoryAllocation.bytesAllocated += totalSize;
    memoryAllocation.bytes += totalSize;
    memoryAllocation.bytesSize = size;

    //allocate memory block
    char* poolMemory = (char*)malloc(totalSize);
    if (poolMemory == nullptr)
    {
        throw std::bad_alloc();
    }

    //setup header data
    Header* headerPtr = (Header*)poolMemory;
    headerPtr->size = size;
    headerPtr->checkValue = CHECK_VALUE;
    headerPtr->nextHeader = nullptr;

    //update the linked list of allocations
    if (lastAllocation != nullptr)
    {
        lastAllocation->nextHeader = headerPtr;
    }
    headerPtr->previousHeader = lastAllocation;
    lastAllocation = headerPtr;

    //check if its the first one and set it, if its the case
    if (firstAllocation == nullptr)
    {
        firstAllocation = headerPtr;
    }

    //set up footer data
    Footer* footerPtr = (Footer*)(poolMemory + sizeof(Header) + size);
    footerPtr->checkValue = CHECK_VALUE;

    //update memory data
    totalAllocatedMemory += size;

    //return memory block
    void* pStartMemBlock = poolMemory + sizeof(Header);
    return pStartMemBlock;
}

void operator delete(void* poolMemory)
{
    //error check
    if (poolMemory == nullptr)
    {
        std::cout << "poolMemory is nullptr. Nothing to delete" << std::endl;
        return;
    }

    //lock mutex for safety
    std::lock_guard<std::mutex> lock(getAllocationMutex());

    //retrieve header and footer
    Header* headerPtr = (Header*)((char*)poolMemory - sizeof(Header));
    Footer* footerPtr = (Footer*)((char*)poolMemory + headerPtr->size);

    //update linked list of allocations
    if (headerPtr == firstAllocation)
    {
        firstAllocation = headerPtr->nextHeader;
    }
    else if (headerPtr->previousHeader != nullptr)
    {
        headerPtr->previousHeader->nextHeader = headerPtr->nextHeader;
    }

    if (headerPtr == lastAllocation)
    {
        lastAllocation = headerPtr->previousHeader;
    }
    else if (headerPtr->nextHeader != nullptr)
    {
        headerPtr->nextHeader->previousHeader = headerPtr->previousHeader;
    }

    //buffer overflow checks
    if (headerPtr->checkValue != CHECK_VALUE)
    {
        std::cout << "header check value doesn't match. Buffer Overflow!" << std::endl;
        return;
    }

    if (footerPtr->checkValue != CHECK_VALUE)
    {
        std::cout << "footer check value doesn't match. Buffer Overflow!" << std::endl;
        return;
    }

    //update the memory data
    totalAllocatedMemory -= headerPtr->size;
    memoryAllocation.bytesSize = 0;
    memoryAllocation.bytesDeallocated += sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);
    memoryAllocation.bytes -= sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);

    //free the memory and set to null
    free(headerPtr);
    headerPtr = nullptr;
}