#pragma once

namespace core
{

//-----------------------------------------------------------------------------
// CMurmurHash2A, by Austin Appleby
// From https://code.google.com/p/smhasher/source/browse/trunk/MurmurHash2.cpp
// This is a sample implementation of MurmurHash2A designed to work 
// incrementally.

// Usage - 

// CMurmurHash2A hasher
// hasher.Begin(seed);
// hasher.Add(data1,size1);
// hasher.Add(data2,size2);
// ...
// hasher.Add(dataN,sizeN);
// uint32_t hash = hasher.End()

#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

class MurmurHash2A
{
public:
    void Begin(uint32_t seed = 0)
    {
        hash = seed;
        tail = 0;
        count = 0;
        size = 0;
    }

    void Add(const void *inputData, int len)
    {
        const uint8_t *data = (uint8_t *)inputData;
        size += len;

        MixTail(data, len);

        while (len >= 4) {
            uint32_t k = *(uint32_t*)data;

            mmix(hash, k);

            data += 4;
            len -= 4;
        }

        MixTail(data, len);
    }

    uint32_t End()
    {
        mmix(hash, tail);
        mmix(hash, size);

        hash ^= hash >> 13;
        hash *= m;
        hash ^= hash >> 15;

        return hash;
    }

private:
    static const uint32_t m = 0x5bd1e995;
    static const int r = 24;

    void MixTail(const unsigned char *&data, int &len)
    {
        while (len && ((len < 4) || count)) {
            tail |= (*data++) << (count * 8);

            count++;
            len--;

            if (count == 4) {
                mmix(hash, tail);
                tail = 0;
                count = 0;
            }
        }
    }

    uint32_t hash;
    uint32_t tail;
    uint32_t count;
    uint32_t size;
};

#undef mmix

}   // core