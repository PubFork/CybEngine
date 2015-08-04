#pragma once
#include <cstdlib>
#include <cstdint>

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
// Original source at https://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp

class HasherMurmur3 {
public:
	HasherMurmur3( uint32_t initSeed = 0 ) {
		seed = initSeed;
		hash = 0;
		size = 0;
		finalized = false;
	}

	void Add( const void *input, uint32_t length ) {
		size += length;

		const uint32_t c1 = 0xcc9e2d51;
		const uint32_t c2 = 0x1b873593;

		const uint8_t *data = (uint8_t *)input;
		const int32_t numBlocks = length / 4;
		uint32_t h1 = seed;

		//----------
		// body
		const uint32_t *blocks = (const uint32_t *)(data + numBlocks * 4);
		for ( int32_t i = -numBlocks; i; i++ ) {
			uint32_t k1 = GetBlock( blocks, i );
			k1 *= c1;
			k1 = _rotl( k1, 15 );
			k1 *= c2;

			h1 ^= k1;
			h1 = _rotl( h1, 13 );
			h1 = h1 * 5 + 0xe6546b64;
		}

		//----------
		// tail
		const uint8_t *tail = (const uint8_t *)(data + numBlocks * 4);

		uint32_t k1 = 0;

		switch ( length & 3 ) {
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
			k1 *= c1; 
			k1 = _rotl( k1, 15 ); 
			k1 *= c2; 
			h1 ^= k1;
		};

		//----------
		// finalization
		h1 ^= length;
		h1 = Mix( h1 );
		hash ^= h1;
	}

	template <class T>
	void Add( T value ) {
		Add( &value, sizeof( T ) );
	}

	uint32_t Hash() {
		if ( !finalized ) {
			Finalize();
		}

		return hash;
	}

private:
	uint32_t GetBlock( const uint32_t *p, int i ) {
		return p[i];
	}

	void Finalize() {
		if ( !finalized ) {
			hash ^= size;
			hash = Mix( hash );
		}
	}

	uint32_t Mix( uint32_t h ) {
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;

		return h;
	}

	uint32_t hash;
	uint32_t seed;
	uint32_t size;
	bool finalized;
};