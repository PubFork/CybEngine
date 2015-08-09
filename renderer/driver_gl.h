#pragma once
#include <cstdint>
#include <GL/glew.h>
#include "core/memory.h"
#include "vertex_layout.h"

namespace cyb {

struct VertexBufferGL {
	void Create( const SharedRef<memory_t> mem, const VertexLayoutHandle layoutHandle );
	void Destroy();

	GLuint id;
	GLsizei size;
	VertexLayoutHandle vertexLayout;
};

struct IndexBufferGL {
	void Create( const SharedRef<memory_t> mem );
	void Destroy();

	GLuint id;
	GLsizei size;
};

struct ShaderGL {
	void Create( const SharedRef<memory_t> mem, GLenum shaderType );
	void Destroy();

	GLuint id;
	GLenum type;
};

struct UniformGL {
	enum Enum {
		View,          // u_view
		InvView,       // u_invView
		Proj,          // u_proj
		InvProj,       // u_invProj
		ViewProj,      // u_viewProj
		InvViewProj,   // u_invViewProj
		Model,         // u_model
		ModelView,     // u_modelView
		ModelViewProj, // u_modelViewProj
		Count
	};

	GLenum type;
	GLint  location;
	GLint  num;
};

struct ProgramGL {
	ProgramGL();
	void Create( const ShaderGL &vertexShader, const ShaderGL &fragmentShader );
	void Destroy();

	void BindAttributes(const VertexLayout &layout);

	GLuint id;
	GLint attributes[VertexAttribute::Count + 1];
	UniformGL predefined[UniformGL::Count];
	uint16_t numPredefined;
};

}	// namespace cyb