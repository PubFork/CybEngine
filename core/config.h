#pragma once

#define CYB_CONFIG_USE_32BIT_INDICES	0

namespace cyb {

static const size_t MAX_VERTEX_BUFFERS = 1024;
static const size_t MAX_INDEX_BUFFERS = 1024;
static const size_t MAX_VERTEX_LAYOUTS = 64;
static const size_t MAX_SHADER_PROGRAMS = 64;
static const size_t LOG_HISTORY_SIZE = 64;

}	// namespace cyb

#if CYB_CONFIG_USE_32BIT_INDICES
#define INDEX_MAX		UINT32_MAX
#define INDEX_C			UINT32_C
using index_t = unsigned int;
#else
#define INDEX_MAX		UINT16_MAX
#define INDEC_			UINT16_C
using index_t = unsigned short;
#endif