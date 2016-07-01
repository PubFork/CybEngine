#pragma once

#define RETURN_FALSE_IF(expression)     if (expression) { return false; }
#define RETURN_NULL_IF(expression)      if (expression) { return NULL; }

template <typename T> 
inline T Min(const T a, const T b) 
{ 
    return a < b ? a : b; 
}

template <typename T>
inline T Max(const T a, const T b) 
{
    return a > b ? a : b;
}

template <typename T>
inline T Clamp(const T x, const T min, const T max)
{ 
    return x < min ? min : (x > max ? max : x);
}

template <typename T>
inline T Saturate(const T &x)
{
    T result = Clamp(x, T(0), T(1));
    return result;
}