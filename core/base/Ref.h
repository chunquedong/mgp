#ifndef REF_H_
#define REF_H_


#if _DEBUG
#define GP_USE_REF_TRACE
#endif

#ifndef NO_THREAD_SAFE
#include <atomic>
#endif

namespace mgp
{

class Refable;

class WeakRefBlock {
    friend class Refable;
#if NO_THREAD_SAFE
    unsigned int _weakRefCount;
#else
    std::atomic<unsigned int> _weakRefCount;
#endif

    Refable* _pointer;
public:
    WeakRefBlock();
    ~WeakRefBlock();

    Refable* lock();

    void addRef();
    void release();
};

/**
 * Defines the base class for game objects that require lifecycle management.
 *
 * This class provides reference counting support for game objects that
 * contain system resources or data that is normally long lived and
 * referenced from possibly several sources at the same time. The built-in
 * reference counting eliminates the need for programmers to manually
 * keep track of object ownership and having to worry about when to
 * safely delete such objects.
 */
class Refable
{
public:

    /**
     * Increments the reference count of this object.
     *
     * The release() method must be called when the caller relinquishes its
     * handle to this object in order to decrement the reference count.
     */
    void addRef();

    /**
     * Decrements the reference count of this object.
     *
     * When an object is initially created, its reference count is set to 1.
     * Calling addRef() will increment the reference and calling release()
     * will decrement the reference count. When an object reaches a
     * reference count of zero, the object is destroyed.
     */
    void release();

    /**
     * Returns the current reference count of this object.
     *
     * @return This object's reference count.
     */
    unsigned int getRefCount() const;

    void _setRefCount(int rc);

    WeakRefBlock* getWeakRefBlock();
protected:

    /**
     * Constructor.
     */
    Refable();

    /**
     * Copy constructor.
     * 
     * @param copy The Refable object to copy.
     */
    Refable(const Refable& copy);

    /**
     * Destructor.
     */
    virtual ~Refable();

private:
#if NO_THREAD_SAFE
    unsigned int _refCount;
#else
    std::atomic<unsigned int> _refCount;
#endif
    WeakRefBlock* _weakRefBlock;

    // Memory leak diagnostic data (only included when GP_USE_MEM_LEAK_DETECTION is defined)
#ifdef GP_USE_REF_TRACE
    friend class Game;
    friend void* trackRef(Refable* ref);
    friend void untrackRef(Refable* ref);
    static void printLeaks();
    Refable* _next;
    Refable* _prev;
#endif
};

}

#endif
