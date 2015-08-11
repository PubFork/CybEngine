#pragma once
#include <stdint.h>
#include <assert.h>

#define BYTES( x )		( (x) )
#define KILOBYTES( x )	( (x) << 10 )
#define MEGABYTES( x )	( (x) << 20 )
#define GIGABYTES( x )	( (x) << 30 )

// 16-byte aligned heap allocation
void *Mem_Alloc16( size_t numBytes );
void Mem_Free16( void *memory );

/** Memory allocator interface. */
class IAllocator {
public:
	enum { DefaultAlignment = 16 };

	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) = 0;
	virtual void Free( void *memory ) = 0;
	virtual void Flush() = 0;
};

/** System malloc/free allocator. */
class CrtAllocator : public IAllocator {
public:
	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) final;
	virtual void Free( void *memory ) final;
	virtual void Flush() final {}
};

/** 
 * Simple linear allocator. 
 * Free() does nothing, use Flush() to reset the allocator.
 */
class LinearAllocator : public IAllocator {
public:
	LinearAllocator( size_t memoryPoolSize );
	virtual ~LinearAllocator() final;
	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) final;
	virtual void Free( void * ) final {}
	virtual void Flush() final;

private:
	uint8_t *top;
	uint8_t *end;
	void *memoryPool;
};

// Internal functions for smart pointers, should not be used directly.
namespace SharedPointerInternals {

inline void DefaultDeleter( void *object ) {
	delete object;
}

class BasePointer {
public:
	BasePointer( void *obj ) {
		object = obj;
		sharedReferenceCount = 1;
		weakReferenceCount = 1;
	}

	uint32_t AddSharedRef() { 
		return sharedReferenceCount++; 
	}

	uint32_t ReleaseShared() {
		if ( --sharedReferenceCount == 0) {
			DeleteObject( object );		// delete object
			delete this;				// delete this
			return 0;
		}

		return sharedReferenceCount;
	}

	virtual void DeleteObject( void * ) = 0;

private:
	uint16_t sharedReferenceCount;
	uint16_t weakReferenceCount;
	void *object;
};

template <class DeleterType>
class BasePointerDel : public BasePointer {
public:
	BasePointerDel( void *obj, DeleterType deleter ) :
		BasePointer( obj ),
		objectDeleter( deleter ) {
	}

	virtual void DeleteObject( void *memory ) final {
		objectDeleter( memory );
	}
	
private:
	DeleterType objectDeleter;
};

}	// namespace SharedPointerInternals

/** SharedRef is a non-nullable, non-intrusive reference counted object reference. */
template <class ObjectType>
class SharedRef {
public:
	/**
	 * Construct a shared reference that owns the specified object.
	 * Must not be nullptr.
	 * @param	obj			Object this shared reference retain reference to.
	 */
	template <class OtherType>
	explicit SharedRef( OtherType *obj ) {
		assert( obj );
		object = obj;
		basePtr = new SharedPointerInternals::BasePointerDel<void (*)(void*)>( obj, &SharedPointerInternals::DefaultDeleter );
	}

	/**
	* Construct a shared reference that owns the specified object using a custom deleter.
	* Must not be nullptr.
	* @param	obj			Object this shared reference retain reference to.
	* @param	deleter		Custom deleter function.
	*/
	template <class OtherType, class DeleterType>
	explicit SharedRef( OtherType *obj, DeleterType deleter ) {
		assert( obj );
		object = obj;
		basePtr = new SharedPointerInternals::BasePointerDel<DeleterType>( obj, deleter );
	}

	/**
	 * Construct a shared reference copy of another shared reference.
	 * @param	ref			Shared reference to copy from.
	 */
	SharedRef( const SharedRef &ref ) {
		assert( ref.IsValid() );
		object = ref.object;
		basePtr = ref.basePtr;
		basePtr->AddSharedRef();
	}

	/**
	 * Destructor. If this shared reference is the only one retaining reference
	 * to the object, the object will be destructed and freed.
	 */
	~SharedRef() {
		basePtr->ReleaseShared();
	}

	/**
	 * This shared reference will be replaced by the specified shared reference.
	 * The object currently referenced to by this shared reference will no longer
	 * be referenced and will be deleted if there are no other references.
	 * @param	ref			Shared reference used to assign this one.
	 * @return A reference to this.
	 */
	SharedRef &operator=( const SharedRef &ref ) {
		assert( ref.IsValid() );
		basePtr->ReleaseShared();

		object = ref.object;
		basePtr = ref.basePtr;
		basePtr->AddSharedRef();
		return *this;
	}

	/**
	 * Get a reference to the object managed by this shared reference.
	 * @return A reference to the object.
	 */
	ObjectType &Get() const {
		assert( IsValid() );
		return *object;
	}

	/** 
	 * Dereference operator returns a reference to the object this shared reference point to.
	 * @return A reference to the object.
	 */
	ObjectType &operator*() const {
		assert( IsValid() );
		return *object; 
	}

	/**
	 * Arrow operator returns a pointer to the object his shared reference points to.
	 * @return A pointer to the object.
	 */
	ObjectType *operator->() const {
		assert( IsValid() );
		return object;
	}

private:
	bool IsValid() const { return (object != nullptr); }

	ObjectType *object;
	SharedPointerInternals::BasePointer *basePtr;
};

/** Container for SharedMem_* functions. */
struct memory_t {
	void *buffer;
	size_t size;
};

SharedRef<memory_t> SharedMem_Alloc( size_t numBytes );
SharedRef<memory_t> SharedMem_MakeRef( const void *refMem, size_t size );
SharedRef<memory_t> SharedMem_Alloc( SharedRef<IAllocator> allocator, size_t numBytes );

// type-safe allocations using custom allocators
void *operator new  (size_t size, SharedRef<IAllocator> allocator, uint32_t count = 1, uint32_t alignment = IAllocator::DefaultAlignment);
void *operator new[]( size_t size, SharedRef<IAllocator> allocator, uint32_t count = 1, uint32_t alignment = IAllocator::DefaultAlignment );
void operator delete  (void *object, SharedRef<IAllocator> allocator, uint32_t, uint32_t);
void operator delete[]( void *object, SharedRef<IAllocator> allocator, uint32_t, uint32_t );