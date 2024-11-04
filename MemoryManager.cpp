#include "MemoryManager.h"
#include <iostream>
#include <mutex>

const unsigned int GUARD_VALUE = 0xDEADBEEF;
size_t totalAllocatedMemory = 0;

Header* firstAllocation{ nullptr };
Header* lastAllocation{ nullptr };

MemoryAllocation memoryAllocation;

// Mutex for thread-safe operations
std::mutex memoryMutex;

void* operator new(size_t size)
{
    std::lock_guard<std::mutex> lock(memoryMutex); // Lock for thread safety

    size_t totalSize = sizeof(Header) + sizeof(Footer) + size;

    memoryAllocation.bytesAllocated += totalSize;
    memoryAllocation.bytes += totalSize;

    char* poolMemory = (char*)malloc(totalSize);

    if (poolMemory == nullptr)
    {
        throw std::bad_alloc();
    }

    Header* headerPtr = (Header*)poolMemory;
    headerPtr->size = size;
    headerPtr->guardValue = GUARD_VALUE;
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
    footerPtr->guardValue = GUARD_VALUE;

    totalAllocatedMemory += size;

    void* pStartMemBlock = poolMemory + sizeof(Header);
    return pStartMemBlock;
}

void operator delete(void* poolMemory)
{
    if (poolMemory == nullptr)
    {
        std::cout << "poolMemory is nullptr. Nothing to delete" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(memoryMutex); // Lock for thread safety

    Header* headerPtr = (Header*)((char*)poolMemory - sizeof(Header));
    Footer* footerPtr = (Footer*)((char*)poolMemory + headerPtr->size);

    if (headerPtr == firstAllocation)
    {
        firstAllocation = headerPtr->nextHeader;
    }
    else
    {
        if (headerPtr->previousHeader != nullptr) { // Protect against nullptr dereference
            headerPtr->previousHeader->nextHeader = headerPtr->nextHeader;
        }
    }

    if (headerPtr == lastAllocation)
    {
        lastAllocation = headerPtr->previousHeader;
    }
    else
    {
        if (headerPtr->nextHeader != nullptr) { // Protect against nullptr dereference
            headerPtr->nextHeader->previousHeader = headerPtr->previousHeader;
        }
    }

    if (headerPtr->guardValue != GUARD_VALUE)
    {
        std::cout << "header guard value doesn't match!" << std::endl;
        return;
    }

    if (footerPtr->guardValue != GUARD_VALUE)
    {
        std::cout << "footer guard value doesn't match!" << std::endl;
        return;
    }

    totalAllocatedMemory -= headerPtr->size;

    memoryAllocation.bytesDeallocated += sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);
    memoryAllocation.bytes -= sizeof(*headerPtr) + headerPtr->size + sizeof(*footerPtr);

    free(headerPtr); // Free the entire block including header and footer
}
