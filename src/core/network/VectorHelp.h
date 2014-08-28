#pragma once
//
//  VectorHelpers.h
//
#include <algorithm>
#include <vector>


// This is incomplete, and will NOT work for strings

template <class T>
static bool compareVectors(std::vector<T> a, std::vector<T> b)
{
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    return (a == b);
}