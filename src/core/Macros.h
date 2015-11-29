#pragma once

#define BIT(x)              (1ULL << (x))

#define BYTES(x)            (x)
#define KILOBYTES(x)        ((x) << 10ULL)
#define MEGABYTES(x)        ((x) << 20ULL)
#define GIGABYTES(x)        ((x) << 30ULL)

// get the size of the content of a std::vector in bytes
#define VECTOR_BYTESIZE(v)  (sizeof(v[0]) * v.size())