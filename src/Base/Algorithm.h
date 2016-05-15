#pragma once

#define BIT(x)              (1ULL << (x))

/*
#define BYTES(x)            (x)
#define KILOBYTES(x)        ((x) << 10ULL)
#define MEGABYTES(x)        ((x) << 20ULL)
#define GIGABYTES(x)        ((x) << 30ULL)
*/

// get the size of the content of a stl container in bytes
//#define CONTAINER_CONTENT_SIZE(c)   (sizeof(decltype(c)::value_type) * c.size())

// small swapper for std::for_each without having to specify begin- and end iterator
//#define FOR_EACH(element, function) std::for_each(std::begin(element), std::end(element), function);

// arithmic math functions
template <class T> T Min(T a, T b) { return a < b ? a : b; }
template <class T> T Max(T a, T b) { return a > b ? a : b; }
template <class T> T Clamp(T x, T min, T max) { return x < min ? min : (x > max ? max : x); }