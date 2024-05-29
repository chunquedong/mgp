
#include <cstdio>
#include <cstdarg>

#ifdef GP_USE_MEM_LEAK_DETECTION

#include <new>
#include <exception>
#include <thread>
#include <mutex>

bool __trackStackTrace = false;
char* getStackTrace();

struct MemoryAllocationRecord
{
    void *address;          // address returned to the caller after allocation
    unsigned int size;              // size of the allocation request
    int line;                       // source line of the allocation request
    const char* file;               // source file of allocation request
    MemoryAllocationRecord* next;
    MemoryAllocationRecord* prev;
    const char* stackTraceStr;
    uint64_t checkCode;
};

MemoryAllocationRecord* __memoryAllocations = 0;
int __memoryAllocationCount = 0;

static std::mutex& getMemoryAllocationMutex()
{
    static std::mutex m;
    return m;
}

void* debugAlloc(std::size_t size, const char* file, int line);
void debugFree(void* p);

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

void* operator new (std::size_t size, const char* file, int line)
{
    return debugAlloc(size, file, line);
}

void* operator new[] (std::size_t size, const char* file, int line)
{
    return operator new (size, file, line);
}

void* operator new (std::size_t size) throw(std::bad_alloc)
{
    return operator new (size, "", 0);
}

void* operator new[] (std::size_t size) throw(std::bad_alloc)
{
    return operator new (size, "", 0);
}

void* operator new (std::size_t size, const std::nothrow_t&) throw()
{
    return operator new (size, "", 0);
}

void* operator new[] (std::size_t size, const std::nothrow_t&) throw()
{
    return operator new (size, "", 0);
}

void operator delete (void* p) throw()
{
    debugFree(p);
}

void operator delete[] (void* p) throw()
{
    operator delete (p);
}

void operator delete (void* p, const char* file, int line) throw()
{
    operator delete (p);
}

void operator delete[] (void* p, const char* file, int line) throw()
{
    operator delete (p);
}

// Include Base.h (needed for logging macros) AFTER new operator impls
#include "base/Base.h"

void* debugAlloc(std::size_t size, const char* file, int line)
{
    // Allocate memory + size for a MemoryAlloctionRecord
    unsigned char* mem = (unsigned char*)malloc(size + sizeof(MemoryAllocationRecord));
    assert(mem);

    MemoryAllocationRecord* rec = (MemoryAllocationRecord*)mem;

    // Move memory pointer past record
    mem += sizeof(MemoryAllocationRecord);

    std::lock_guard<std::mutex> lock(getMemoryAllocationMutex());
    rec->address = mem;
    rec->size = (unsigned int)size;
    rec->file = file;
    rec->line = line;
    rec->next = __memoryAllocations;
    rec->prev = 0;
    rec->checkCode = 0xABCD;
    
    if (__trackStackTrace) {
        rec->stackTraceStr = getStackTrace();
    }
    else {
        rec->stackTraceStr = NULL;
    }

    if (__memoryAllocations)
        __memoryAllocations->prev = rec;
    __memoryAllocations = rec;
    ++__memoryAllocationCount;

    return mem;
}

void debugFree(void* p)
{
    if (p == 0)
        return;

    // Backup passed in pointer to access memory allocation record
    void* mem = ((unsigned char*)p) - sizeof(MemoryAllocationRecord);

    MemoryAllocationRecord* rec = (MemoryAllocationRecord*)mem;

    // Sanity check: ensure that address in record matches passed in address
    if (rec->address != p || rec->checkCode != 0xABCD)
    {
        GP_ASSERT(rec->address);
        mgp::print("[memory] CORRUPTION: Attempting to free memory address with invalid memory allocation record.\n");
        return;
    }
    rec->checkCode = 0;

    // Link this item out
    std::lock_guard<std::mutex> lock(getMemoryAllocationMutex());
    if (__memoryAllocations == rec)
        __memoryAllocations = rec->next;
    if (rec->prev)
        rec->prev->next = rec->next;
    if (rec->next)
        rec->next->prev = rec->prev;
    --__memoryAllocationCount;

    // Free the address from the original alloc location (before mem allocation record)
    free(mem);
}

extern void printMemoryLeaks()
{
    // Dump general heap memory leaks
    if (__memoryAllocationCount == 0)
    {
        mgp::print("[memory] All HEAP allocations successfully cleaned up (no leaks detected).\n");
    }
    else
    {
        mgp::print("[memory] WARNING: %d HEAP allocations still active in memory.\n", __memoryAllocationCount);
        MemoryAllocationRecord* rec = __memoryAllocations;
        while (rec)
        {
            mgp::print("[memory] LEAK: HEAP allocation leak at address %#x of size %d from line %d in file '%s'.\n", rec->address, rec->size, rec->line, rec->file);
            if (rec->stackTraceStr) {
                mgp::print(rec->stackTraceStr);
                free((void*)rec->stackTraceStr);
            }
            rec = rec->next;
        }
    }
}

#endif



#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib,"dbghelp.lib")

char* getStackTrace()
{
    unsigned int   i;
    void* stack[100];
    unsigned short frames;
    SYMBOL_INFO* symbol;
    HANDLE         process;
    int strAlloc;

    process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    frames = CaptureStackBackTrace(0, 100, stack, NULL);
    symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    strAlloc = 1024;
    char* buffer = (char*)malloc(strAlloc);
    int pos = 0;
    int newAllocSize = 0;
    for (i = 0; i < frames; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        if (strAlloc - pos < 256) {
            strAlloc = strAlloc * 1.5;
            buffer = (char*)realloc(buffer, strAlloc);
        }

        pos += snprintf(buffer + pos, strAlloc - pos, "  %i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
    }

    free(symbol);
    return buffer;
}

#elif defined(__EMSCRIPTEN__)

char* getStackTrace()
{
    return NULL;
}

#else

#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

char* getStackTrace() {
    void* array[100];
    int size;
    char** strings;
    size_t i;

    int strSize = 0;
    size_t strAlloc;
    size_t nameSize;
    char* str;
    char* name;

    str = (char*)malloc(256);
    if (str == NULL) {
        printf("bad alloc\n");
        abort();
    }
    strAlloc = 255;

    size = backtrace(array, 100);
    strings = backtrace_symbols(array, size);
    if (NULL == strings) {
        str[strSize] = 0;
        return str;
    }

    for (i = 0; i < size; i++) {
        name = strrchr(strings[i], '/');
        if (!name) continue;

        nameSize = strlen(name) + 1;
        if (strSize + nameSize > strAlloc) {
            strAlloc = (strSize + nameSize) * 3 / 2;
            str = (char*)realloc(str, strAlloc + 1);
            if (str == NULL) {
                printf("bad alloc\n");
                abort();
            }
        }
        strcpy(str + strSize, name);
        strSize += nameSize;
        str[strSize - 1] = ',';
        str[strSize] = '\0';
    }
    free(strings);

    str[strSize] = 0;
    return str;
}

#endif
