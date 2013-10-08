#pragma once

#include <cassert>
#include <algorithm>

template<typename T> class com_ptr
{

public:
    
    void swap(com_ptr& that) { std::swap(m, that.m); }

    com_ptr() : m(nullptr) {};
    explicit com_ptr(T* p) : m(p) {}
    com_ptr(const com_ptr& that) : m(that.m) { if (m) m->AddRef(); }
    com_ptr(com_ptr&& that) : m(nullptr) { swap(that); }
        
    ~com_ptr() { if (m) m->Release(); };
    
    com_ptr& operator=(T* p) { com_ptr tmp(p); swap(tmp); return *this; }
    com_ptr& operator=(const com_ptr& that) { com_ptr tmp(that); swap(tmp); return *this; }
    com_ptr& operator=(com_ptr&& that) { swap(that); return *this; }
    
    T* operator->() const { assert(m); return m; }
    
    // Potentially dangerous automatic conversions
    operator T**() { assert(!m); return &m; } // Check assumes usage in functions that write the pointer, accordingly we require it to be null 
    operator T*() const { assert(m); return m; } // Check assumes usage by dereference, accordingly we require to be non-null
    // Required to prevent invocation of pointer automatic conversions
    operator const bool() { return m != nullptr; }       
    operator const bool() const { return m != nullptr; }
    const bool operator!() const { return m == nullptr; }

private:

    T* m;

};