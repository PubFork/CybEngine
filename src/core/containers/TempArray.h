#pragma once
#include "Memory.h"

template <class T>
class TempArray
{
public:
	TempArray( size_t num )
	{
		this->num = num;
		buffer = (T *)Mem_Alloc( sizeof( T ) * num );
	}

	~TempArray()
	{
		Mem_Free( buffer );
	}

	T *Get() { return buffer; }
	const T *Get() const { return buffer; }
	size_t Size() const { return sizeof( T ) * num; }
	size_t Num() const { return num; }
	T &operator[]( size_t index ) { assert( index < num ); return buffer[index]; }
	const T &operator[]( size_t index ) const { assert( index < num ); return buffer[index]; }

private:
	T *buffer;
	size_t num;
};