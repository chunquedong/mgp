#include "base/Base.h"
#include "base/Ref.h"
//#include "platform/Toolkit.h"
#include <mutex>

namespace mgp
{
std::mutex traceLock;
#ifdef GP_USE_REF_TRACE
void* trackRef(Refable* ref);
void untrackRef(Refable* ref);
#endif

Refable::Refable() :
    _refCount(1), _weakRefBlock(NULL)
{
#ifdef GP_USE_REF_TRACE
    trackRef(this);
#endif
}

Refable::Refable(const Refable& copy) :
    _refCount(1)
{
#ifdef GP_USE_REF_TRACE
    trackRef(this);
#endif
}

Refable::~Refable()
{
    GP_ASSERT(_refCount == 0);
    _refCount = 1000000;
#ifdef GP_USE_REF_TRACE
    untrackRef(this);
#endif
}

void Refable::addRef()
{
    GP_ASSERT(_refCount > 0 && _refCount < 1000000);
    ++_refCount;
}

void Refable::release()
{
    GP_ASSERT(_refCount > 0 && _refCount < 1000000);
    if ((--_refCount) <= 0)
    {
        if (_weakRefBlock) {
            std::lock_guard<std::mutex> guard(traceLock);
            if (_weakRefBlock->_weakRefCount == 0) {
                delete _weakRefBlock;
            }
            else {
                _weakRefBlock->_pointer = NULL;
            }
        }
        delete this;
    }
}

void Refable::_setRefCount(int rc) {
    _refCount = rc;
}

unsigned int Refable::getRefCount() const
{
    return _refCount;
}

////////////////////////////////////////////////////////////////////////////////////////

WeakRefBlock* Refable::getWeakRefBlock() {
    if (_weakRefBlock) return _weakRefBlock;
    std::lock_guard<std::mutex> guard(traceLock);
    if (!_weakRefBlock) {
        _weakRefBlock = new WeakRefBlock();
        _weakRefBlock->_pointer = this;
    }
    return _weakRefBlock;
}

WeakRefBlock::WeakRefBlock() : _weakRefCount(0), _pointer(NULL) {
    
}
WeakRefBlock::~WeakRefBlock() {
    GP_ASSERT(_weakRefCount == 0);
    _weakRefCount = 1000000;
    _pointer = NULL;
}

void WeakRefBlock::addRef() {
    GP_ASSERT(_weakRefCount < 1000000);
    ++_weakRefCount;
}

void WeakRefBlock::release() {
    GP_ASSERT(_weakRefCount > 0 && _weakRefCount < 1000000);
    if ((--_weakRefCount) <= 0)
    {
        std::lock_guard<std::mutex> guard(traceLock);
        if (!_pointer) {
            delete this;
        }
    }
}

Refable* WeakRefBlock::lock() {
    std::lock_guard<std::mutex> guard(traceLock);
    if (_pointer) {
        _pointer->addRef();
        return _pointer;
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////

#ifdef GP_USE_REF_TRACE

Refable* __refAllocations = 0;
int __refAllocationCount = 0;

void Refable::printLeaks()
{
    std::lock_guard<std::mutex> guard(traceLock);
    // Dump Refable object memory leaks
    if (__refAllocationCount == 0)
    {
        print("[memory] All Refable objects successfully cleaned up (no leaks detected).\n");
    }
    else
    {
        print("[memory] WARNING: %d Refable objects still active in memory.\n", __refAllocationCount);
        for (Refable* rec = __refAllocations; rec != NULL; rec = rec->_next)
        {
            Refable* ref = rec;
            GP_ASSERT(ref);
            const char* type = typeid(*ref).name();
            print("[memory] LEAK: Refable object '%s' still active with reference count %d.\n", (type ? type : ""), ref->getRefCount());
        }
    }
}

void* trackRef(Refable* ref)
{
    std::lock_guard<std::mutex> guard(traceLock);
    GP_ASSERT(ref);

    // Create memory allocation record.
    Refable* rec = ref;
    rec->_next = __refAllocations;
    rec->_prev = NULL;

    if (__refAllocations)
        __refAllocations->_prev = rec;
    __refAllocations = rec;
    ++__refAllocationCount;

    return rec;
}

void untrackRef(Refable* ref)
{
    std::lock_guard<std::mutex> guard(traceLock);
    Refable* rec = ref;

    // Link this item out.
    if (__refAllocations == rec)
        __refAllocations = rec->_next;
    if (rec->_prev)
        rec->_prev->_next = rec->_next;
    if (rec->_next)
        rec->_next->_prev = rec->_prev;

    --__refAllocationCount;
}

#endif

}
