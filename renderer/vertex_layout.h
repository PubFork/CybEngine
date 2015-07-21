#pragma once

#include <cstdint>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <type_traits> 
#include "core/config.h"
#include "core/handles.h"

namespace cyb {

struct VertexAttribute {
	enum Enum {
		Position,			// a_position
		Normal,				// a_normal
		Color0,				// a_color0
		Color1,				// a_color1
		Indices,			// a_indices
		TexCoord0,			// a_texcoord0
		TexCoord1,			// a_texcoord1
		TexCoord2,			// a_texcoord2
		TexCoord3,			// a_texcoord3
		Count
	};
};

struct AttributeType {
	enum Enum {
		Uint8,
		Uint16,
		HalfFloat,
		Float
	};
};

struct VertexLayoutEntry {
	VertexAttribute::Enum attribute;
	AttributeType::Enum type;
	uint16_t num;
	bool normalized;
};

const char *GetVertexAttributeName( VertexAttribute::Enum attrib );
VertexAttribute::Enum GetVertexAttributeEnum( const char *attrib );
const char *GetAttributeTypeName( AttributeType::Enum type );

struct VertexLayout {
	explicit VertexLayout() {}
	VertexLayout( const std::initializer_list<VertexLayoutEntry> &initList );
	
	void SetLayout( const std::initializer_list<VertexLayoutEntry> &initList );
	void Reset();

	uint32_t hash;
	uint16_t stride;
	uint16_t numEntries;
	uint16_t offset[VertexAttribute::Count];			// Keep offsets outside VertexLayoutEntry sence it's not defined by user
	VertexLayoutEntry entries[VertexAttribute::Count];
};

class VertexLayoutCache {
public:
	VertexLayoutCache() = default;
	~VertexLayoutCache() = default;

	void Init();
	VertexLayoutHandle Find( uint32_t hash ) const;
	void Add( VertexBufferHandle vertexBuffer, VertexLayoutHandle vertexLayout, uint32_t hash );
	VertexLayoutHandle Release( VertexBufferHandle vertexBuffer );

private:
	std::unordered_map<uint32_t, VertexLayoutHandle> m_layouts;
	uint16_t m_refereceCount[MAX_VERTEX_LAYOUTS];
	VertexLayoutHandle m_vertexBufferReference[MAX_VERTEX_BUFFERS];
};

}	// namespace cyb

enum class vertexAttrib_t : uint8_t {
	Position,			// a_position
	Normal,				// a_normal
	Color0,				// a_color0
	Color1,				// a_color1
	Indices,			// a_indices
	TexCoord0,			// a_texcoord0
	TexCoord1,			// a_texcoord1
	TexCoord2,			// a_texcoord2
	TexCoord3			// a_texcoord3
};

enum class attribType_t : uint8_t {
	Uint8,
	Uint16,
	HalfFloat,
	Float
};

struct vertexLayoutEntry_t {
	vertexAttrib_t vertexAttrib;
	attribType_t attribType;
	size_t numElements;
	bool normalized;
};

//This table might only be valid for OpenGL.
static constexpr uint16_t attribTypeSize[][4] = {
	{  1,  2,  4,  4 },		// attribType_t::Uint8
	{  2,  4,  6,  8 },		// attribType_t::Uint16
	{  2,  4,  6,  8 },		// attribType_t::HalfFloat
	{  4,  8, 12, 16 }		// attribType_t::Float
};

template <vertexAttrib_t attrib, attribType_t type, uint16_t num, bool Tnormalized = false>
struct vertexEntry_t {
	static constexpr vertexAttrib_t vertexAttrib = attrib;
	static constexpr attribType_t attribType = type;
	static constexpr uint16_t numEntries = num;
	static constexpr bool normalized = Tnormalized;
	static constexpr uint16_t size = attribTypeSize[static_cast<std::underlying_type<vertexAttrib_t>::type>( attribType )][num - 1];

	uint16_t offset;
};



/*
inline void vertexLayout_t::Create( const vertexLayoutEntry_t *entries, uint16_t entriesCount ) {
	

	for ( int i = 0; i < entriesCount; i++ ) {
		assert( entries[i].numElements > 0 && entries[i].numElements <= 4 );
		const vertexLayoutEntry_t *entry = &entries[i];

		entryList.push_back( *entry );
		offsetList.push_back( stride );

		hash ^= std::hash<uint32_t>()( (uint32_t)entry->attribType );
		//stride += attribTypeSize[(uint8_t)entry->attribType][entry->numElements];
	}
}
*/