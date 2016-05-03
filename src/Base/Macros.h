#pragma once

#define BIT(x)              (1ULL << (x))

#define BYTES(x)            (x)
#define KILOBYTES(x)        ((x) << 10ULL)
#define MEGABYTES(x)        ((x) << 20ULL)
#define GIGABYTES(x)        ((x) << 30ULL)

// get the size of the content of a stl container in bytes
#define CONTAINER_CONTENT_SIZE(c)   (sizeof(decltype(c)::value_type) * c.size())

// small swapper for std::for_each without having to specify begin- and end iterator
#define FOR_EACH(element, function) std::for_each(std::begin(element), std::end(element), function);

// get value clamped to min and max
template <class T>
T Clamp(const T value, const T minValue, const T maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}
