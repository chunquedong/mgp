/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#ifndef PTR_H_
#define PTR_H_

#include <cstdio>
#include <cstdlib>
#include <type_traits>

#include "Ref.h"

namespace mgp
{

inline void mgp_assert(bool c, const char *msg = "") {
    if (!c) {
        printf("ERROR: %s\n", msg);
        abort();
    }
}

class Refable;

template<typename U>
typename std::enable_if<std::is_base_of_v<Refable, U>, void>::type doFree(U* p) {
    Refable* r = dynamic_cast<Refable*>(p);
    if (r) {
        r->release();
    }
}


template<typename U>
typename std::enable_if<!std::is_base_of_v<Refable, U>, void>::type doFree(U* p) {
    delete p;
}

template<typename T, bool nullable = false>
class UniquePtr {
    T* pointer;

    template <class U, bool nullable2> friend class UniquePtr;

public:
    UniquePtr() : pointer(nullptr) {
        //if (!nullable) static_assert(false, "non-nullable");
    }

    explicit UniquePtr(T* p) : pointer(p) {
        /*if (!nullable) {
            mgp_assert(p, "non-nullable");
        }*/
    }

    ~UniquePtr() {
        clear();
    }

    UniquePtr(const UniquePtr& other) = delete;

    UniquePtr(UniquePtr&& other) {
        if (!nullable) {
            mgp_assert(other.pointer, "non-nullable");
        }
        if (other.pointer) {
            pointer = other.pointer;
            other.pointer = nullptr;
        }
        else {
            pointer = nullptr;
        }
    }

    template <class U>
    UniquePtr(UniquePtr<U>&& other) {
        if (!nullable) {
            mgp_assert(other.pointer, "non-nullable");
        }
        if (other.pointer) {
            pointer = other.pointer;
            other.pointer = nullptr;
        }
        else {
            pointer = nullptr;
        }
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) {
        if (!nullable) {
            mgp_assert(other.pointer, "non-nullable");
        }

        T* toDelete = pointer;

        if (other.pointer) {
            pointer = other.pointer;
            other.pointer = nullptr;
        }
        else {
            pointer = nullptr;
        }

        if (toDelete) {
            doFree(toDelete);
        }
        return *this;
    }

    T* operator->() const { mgp_assert(pointer != nullptr, "try deref null pointer"); return pointer; }

    T* operator->() { mgp_assert(pointer != nullptr, "try deref null pointer"); return pointer; }

    T& operator*() { mgp_assert(pointer != nullptr, "try deref null pointer"); return *pointer; }

    T* get() const { return pointer; }

    bool isNull() { return pointer == nullptr; }

    void clear() {
        doFree(pointer);
        pointer = nullptr;
    }

    T* take() {
        T* p = pointer;
        pointer = nullptr;
        return p;
    }

    void swap(UniquePtr& other) {
        T* p = pointer;
        pointer = other.pointer;
        other.pointer = p;
    }

    template <class U> UniquePtr<U> staticCastTo()
    {
        UniquePtr<U> copy(static_cast<U*>(take()));
        return copy;
    }

    template <class U> UniquePtr<U> dynamicCastTo()
    {
        UniquePtr<U> copy(dynamic_cast<U*>(take()));
        return copy;
    }
};

template <class T>
UniquePtr<T> uniqueFromInstant(T* ptr) {
    ptr->addRef();
    return UniquePtr<T>(ptr);
}

/**
* Intrusive smart pointer. the T must be extend from Ref
*/
template<typename T>
class SharedPtr {
    T* pointer;
    template <class U> friend class SharedPtr;
public:
    SharedPtr() : pointer(nullptr) {
    }

    explicit SharedPtr(T* pointer) : pointer(pointer) {
        //pointer->addRef();
    }

    SharedPtr(const SharedPtr& other) : pointer(other.pointer) {
        if (other.pointer) {
            Refable* refp = dynamic_cast<Refable*>(other.pointer);
            mgp_assert(refp);
            refp->addRef();
        }
    }

    template <class U>
    SharedPtr(SharedPtr<U>& other) : pointer(other.pointer) {
        if (other.pointer) {
            Refable* refp = dynamic_cast<Refable*>(other.pointer);
            mgp_assert(refp);
            refp->addRef();
        }
    }

    virtual ~SharedPtr() {
        clear();
    }

    SharedPtr& operator=(T* other) {
        if (other) {
            Refable* refp = dynamic_cast<Refable*>(other);
            mgp_assert(refp);
            refp->addRef();
        }
        if (pointer) {
            Refable* refp = dynamic_cast<Refable*>(pointer);
            mgp_assert(refp);
            refp->release();
        }
        pointer = other;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (other.pointer) {
            Refable* refp = dynamic_cast<Refable*>(other.pointer);
            mgp_assert(refp);
            refp->addRef();
        }
        if (pointer) {
            Refable* refp = dynamic_cast<Refable*>(pointer);
            mgp_assert(refp);
            refp->release();
        }
        pointer = other.pointer;
        return *this;
    }

    T* operator->() { return pointer; }

    T& operator*() { return *pointer; }

    T* get() const { return pointer; }

    void _set(T* p) { pointer = p; }

    bool isNull() { return pointer == nullptr; }

    void clear() {
        if (pointer) {
            Refable* refp = dynamic_cast<Refable*>(pointer);
            mgp_assert(refp);
            refp->release();
            pointer = nullptr;
        }
    }

    template <class U> SharedPtr<U> staticCastTo()
    {
        SharedPtr<U> copy;
        copy = (static_cast<U*>(get()));
        return copy;
    }

    template <class U> SharedPtr<U> dynamicCastTo()
    {
        SharedPtr<U> copy;
        copy = (dynamic_cast<U*>(get()));
        return copy;
    }

    UniquePtr<T> asUPtr() {
        UniquePtr<T> uptr(pointer);
        pointer = nullptr;
        return uptr;
    }
};


class WeakRefBlock;
template<typename T>
class WeakPtr {
    WeakRefBlock* pointer;
public:
    WeakPtr() : pointer(NULL) {
    }

    WeakPtr(SharedPtr<T>& p) : pointer(NULL) {
        Refable* refp = dynamic_cast<Refable*>(p.get());
        if (refp) {
            pointer = refp->getWeakRefBlock();
            pointer->addRef();
        }
    }

    WeakPtr(T* other) : pointer(NULL) {
        if (other) {
            Refable* refp = dynamic_cast<Refable*>(other);
            pointer = refp->getWeakRefBlock();
            pointer->addRef();
        }
    }

    WeakPtr(const WeakPtr& other) : pointer(other.pointer) {
        if (other.pointer) {
            other.pointer->addRef();
        }
    }

    virtual ~WeakPtr() {
        clear();
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (other.pointer) {
            other.pointer->addRef();
        }
        if (pointer) {
            pointer->release();
        }
        pointer = other.pointer;
        return *this;
    }

    SharedPtr<T> lock() {
        if (!pointer) {
            return SharedPtr<T>();
        }
        SharedPtr<Refable> ptr(pointer->lock());
        return ptr.dynamicCastTo<T>();
    }

    void clear() {
        if (pointer) {
            pointer->release();
            pointer = nullptr;
        }
    }
};



template<typename T> using SPtr = SharedPtr<T>;
template<typename T> using UPtr = UniquePtr<T>;

}
#endif
