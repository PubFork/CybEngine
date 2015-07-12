#include <cassert>
#include <cstring>
#include <functional>
#include "vertex_layout.h"

namespace cyb {

struct VertexTypeInfo {
	const char *name;
	uint16_t size[4];
};

static const VertexTypeInfo s_attributeTypeInfo[] = {
	{ "Uint8",     { 1,  2,  4,  4 } },		// VertexType::Uint8
	{ "Uint16",    { 2,  4,  6,  8 } },		// VertexType::Uint16
	{ "HalfFloat", { 2,  4,  6,  8 } },		// VertexType::HalfFloat
	{ "Float",     { 4,  8, 12, 16 } }		// VertexType::Float
};

static const char *s_vertexAttributeName[] = {
	"a_position",
	"a_normal",
	"a_color0",
	"a_color1",
	"a_indices",
	"a_texcoord0",
	"a_texcoord1",
	"a_texcoord2",
	"a_texcoord3"
};

const char *GetVertexAttributeName( VertexAttribute::Enum attrib ) {
	return s_vertexAttributeName[attrib];
}

VertexAttribute::Enum GetVertexAttributeEnum( const char *attrib ) {
	for ( uint16_t i = 0; i < VertexAttribute::Count; i++ ) {
		if ( strcmp( s_vertexAttributeName[i], attrib ) == 0 ) {
			return static_cast<VertexAttribute::Enum>( i );
		}
	}

	return VertexAttribute::Count;
}

const char *GetAttributeTypeName( AttributeType::Enum type ) {
	return s_attributeTypeInfo[type].name;
}

VertexLayout::VertexLayout(const std::initializer_list<VertexLayoutEntry> &initList) {
	Reset();
	SetLayout( initList );
}

void VertexLayout::Reset() {
	hash = 0;
	stride = 0;
	numEntries = 0;

	memset( offset, 0xff, sizeof( offset ) );
	for ( auto &entry : entries ) {
		entry.attribute = VertexAttribute::Count;
	}
}

void VertexLayout::SetLayout( const std::initializer_list<VertexLayoutEntry> &initList ) {
	assert( initList.size() < VertexAttribute::Count );

	for ( const auto &entry : initList ) {
		assert( entry.num > 0 );
		assert( entry.num <= 4 );
		VertexLayoutEntry *ptr = &entries[numEntries];

		memcpy( ptr, &entry, sizeof( VertexLayoutEntry ) );
		offset[ptr->attribute] = stride;
		stride += s_attributeTypeInfo[ptr->type].size[ptr->num - 1];
		hash ^= std::hash<VertexAttribute::Enum>()( ptr->attribute );
		++numEntries;
	}
}

void VertexLayoutCache::Init() {
	memset( &m_refereceCount, 0, sizeof( m_refereceCount ) );
	memset( &m_vertexBufferReference, 0xff, sizeof( m_vertexBufferReference ) );
}

VertexLayoutHandle VertexLayoutCache::Find( uint32_t hash ) const {
	auto it = m_layouts.find( hash );
	if ( it != m_layouts.end() ) {
		return it->second;
	}

	return INVALID_HANDLE;
}

void VertexLayoutCache::Add( VertexBufferHandle vertexBuffer, VertexLayoutHandle vertexLayout, uint32_t hash ) {
	m_vertexBufferReference[vertexBuffer.index] = vertexLayout;
	m_refereceCount[vertexLayout.index]++;
	m_layouts[hash] = vertexLayout;
}

VertexLayoutHandle VertexLayoutCache::Release( VertexBufferHandle vertexBuffer ) {
	VertexLayoutHandle vertexLayout = m_vertexBufferReference[vertexBuffer.index];

	if ( IsValid( vertexLayout ) ) {
		m_refereceCount[vertexLayout.index]--;
		if ( m_refereceCount[vertexLayout.index] != 0 ) {
			return INVALID_HANDLE;
		}
	}

	return vertexLayout;
}

}	// namespace cyb