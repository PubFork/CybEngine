#pragma once

#include <cstdint>
#include <initializer_list>
#include <unordered_map>
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