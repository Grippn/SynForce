#pragma once

#include <iostream>
#include <fstream>

extern class xlog_impl
{
public:
    template<typename T> xlog_impl& operator<<(const T& x);
    xlog_impl& operator<<(std::ostream& (*pf)(std::ostream&));
private:
    static std::fstream& get(); // singleton log file
} xlog;

template<typename T> xlog_impl& xlog_impl::operator<<(const T& x)
{
    (get() << x).flush(); // flush aggressively in case of crash
#ifndef NDEBUG
    std::cout << x;
#endif
    return *this;
}

#define XSHOW(A) xlog << (A) << " <- " << #A << endl;
