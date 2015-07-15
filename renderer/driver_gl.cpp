#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glm/gtc/type_ptr.hpp>
#include "core/interface.h"
#include "core/config.h"
#include "core/logger.h"
#include "command.h"
#include "driver_gl.h"

#if CYB_CONFIG_USE_32BIT_INDICES
#define INDEX_TYPE_GL	GL_UNSIGNED_INT
#else
#define INDEX_TYPE_GL	GL_UNSIGNED_SHORT
#endif

namespace cyb {

static const GLenum s_attributeType[] = {
	GL_UNSIGNED_BYTE,		// AttributeType::Uint8
	GL_UNSIGNED_SHORT,		// AttributeType::Uint16
	GL_HALF_FLOAT,			// AttributeType::HalfFloat
	GL_FLOAT				// AttributeType::Float
};

static const char *s_predefinedUniformNames[UniformGL::Count] = {
	"u_view",
	"u_invView",
	"u_proj",
	"u_invProj",
	"u_viewProj",
	"u_invViewProj",
	"u_model",
	"u_modelView",
	"u_modelViewProj"
};

struct ExtensionGL {
	enum Enum {
		ARB_debug_output,
		ARB_program_interface_query,
		ARB_timer_query,
		ARB_uniform_buffer_object,
		ATI_meminfo,
		NVX_gpu_memory_info,
		Count
	};

	const char *extension;
	bool supported;
	bool required;
};

static ExtensionGL s_extension[ExtensionGL::Count] = {
	{ "GL_ARB_debug_output",            false, false },
	{ "GL_ARB_program_interface_query", false, true },
	{ "GL_ARB_timer_query",             false, false },
	{ "GL_ARB_uniform_buffer_object",   false, false },
	{ "GL_ATI_meminfo",                 false, false },
	{ "GL_NVX_gpu_memory_info",         false, false }
};

struct RendererDriverGL : public IRendererDriver {
	virtual void Init() final;
	virtual void Shutdown() final;

	virtual void CreateVertexBuffer( const VertexBufferHandle handle, const MemoryPtr mem, const VertexLayoutHandle layoutHandle ) final;
	virtual void DestroyVertexBuffer( const VertexBufferHandle handle ) final;
	virtual void CreateIndexBuffer( const IndexBufferHandle handle, const MemoryPtr mem ) final;
	virtual void DestroyIndexBuffer( const IndexBufferHandle handle ) final;
	virtual void CreateVertexLayout( const VertexLayoutHandle handle, const VertexLayout &layout ) final;
	virtual void DestroyVertexLayout( const VertexLayoutHandle handle ) final;
	virtual void CreateProgram( const ShaderProgramHandle handle, const MemoryPtr vertexShaderMem, const MemoryPtr fragmentShaderMem ) final;
	virtual void DestroyProgram( const ShaderProgramHandle handle ) final;
	virtual void Commit( const CommandBuffer *cbuf ) final;

	VertexBufferGL m_vertexBuffers[MAX_VERTEX_BUFFERS];
	IndexBufferGL m_indexBuffers[MAX_INDEX_BUFFERS];
	VertexLayout m_vertexLayouts[MAX_VERTEX_LAYOUTS];
	ProgramGL m_shaderPrograms[MAX_SHADER_PROGRAMS];
	GLuint m_vao;
};


static RendererDriverGL *s_rendererDriverGL = nullptr;

IRendererDriver *CreateRendererDriverGL() {
	if ( s_rendererDriverGL == nullptr ) {
		s_rendererDriverGL = new RendererDriverGL();
	}

	return s_rendererDriverGL;
}

void DestroyRendererDriverGL() {
	delete s_rendererDriverGL;
	s_rendererDriverGL = nullptr;
}

static const char *GLEnumToString( GLenum e ) {
	switch ( e ) {
	case GL_INT:                                return "int";
	case GL_INT_VEC2:                           return "ivec2";
	case GL_INT_VEC3:                           return "ivec3";
	case GL_INT_VEC4:                           return "ivec4";
	case GL_UNSIGNED_INT:                       return "uint";
	case GL_UNSIGNED_INT_VEC2:                  return "uvec2";
	case GL_UNSIGNED_INT_VEC3:                  return "uvec3";
	case GL_UNSIGNED_INT_VEC4:                  return "uvec4";
	case GL_FLOAT:                              return "float";
	case GL_FLOAT_VEC2:                         return "vec2";
	case GL_FLOAT_VEC3:                         return "vec3";
	case GL_FLOAT_VEC4:                         return "vec4";
	case GL_DOUBLE:                             return "double";
	case GL_DOUBLE_VEC2:                        return "dvec2";
	case GL_DOUBLE_VEC3:                        return "dvec3";
	case GL_DOUBLE_VEC4:                        return "dvec4";
	case GL_FLOAT_MAT2:                         return "mat2";
	case GL_FLOAT_MAT3:                         return "mat3";
	case GL_FLOAT_MAT4:                         return "mat4";
	case GL_SAMPLER_2D:                         return "sampler2D";
	case GL_SAMPLER_3D:                         return "sampler3D";
	case GL_SAMPLER_2D_SHADOW:                  return "sampler2DShadow";
	case GL_IMAGE_1D:                           return "image1D";
	case GL_IMAGE_2D:                           return "image2D";
	case GL_IMAGE_3D:                           return "image3D";
	case GL_IMAGE_CUBE:                         return "imageCube";
	case GL_DEBUG_SOURCE_API_ARB:               return "OpenGL";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     return "Windows";
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   return "Shader compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       return "Third party";
	case GL_DEBUG_SOURCE_APPLICATION_ARB:       return "Application";
	case GL_DEBUG_SOURCE_OTHER_ARB:             return "Other";
	case GL_DEBUG_TYPE_ERROR_ARB:               return "Error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: return "Deprecated behaviour";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  return "Undefined behaviour";
	case GL_DEBUG_TYPE_PORTABILITY_ARB:         return "Portability";
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:         return "Performence";
	case GL_DEBUG_TYPE_MARKER:                  return "Marker";
	case GL_DEBUG_TYPE_PUSH_GROUP:              return "Push group";
	case GL_DEBUG_TYPE_POP_GROUP:               return "Pop group";
	case GL_DEBUG_TYPE_OTHER_ARB:               return "Other";
	case GL_DEBUG_SEVERITY_HIGH_ARB:            return "High";
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:          return "Medium";
	case GL_DEBUG_SEVERITY_LOW_ARB:             return "Low";
	case GL_DEBUG_SEVERITY_NOTIFICATION:        return "Notification";
	default: break;
	}

	return "Unknown";
}

void _stdcall DebugOutputProc( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/ ) {
	CYB_DEBUG( "[", GLEnumToString( source ), "::", GLEnumToString( type ), "::", GLEnumToString( severity ), "::", id, "]: ", message );
	//CYB_DEBUG( "Source=", GLEnumToString( source ), " Type=", GLEnumToString( type ), " Id=", id, " Severity=", GLEnumToString( severity ), " Message=", message );
}

void VertexBufferGL::Create( const std::shared_ptr<Memory> mem, const VertexLayoutHandle layoutHandle ) {
	size = mem->size;
	vertexLayout = layoutHandle;

	glGenBuffers( 1, &id );
	glBindBuffer( GL_ARRAY_BUFFER, id );
	glBufferData( GL_ARRAY_BUFFER, size, mem->buffer, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void VertexBufferGL::Destroy() {
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &id );
}

void IndexBufferGL::Create( const std::shared_ptr<Memory> mem ) {
	size = mem->size;

	glGenBuffers( 1, &id );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, mem->buffer, GL_STATIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void IndexBufferGL::Destroy() {
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &id );
}

void ShaderGL::Create( const std::shared_ptr<Memory> mem, GLenum shaderType ) {
	type = shaderType;
	id = glCreateShader( type );

	const GLchar *string = static_cast<const GLchar *>( mem->buffer );
	const GLint length = mem->size;
	glShaderSource( id, 1, &string, &length );
	glCompileShader( id );

	GLint compiled = 0;
	glGetShaderiv( id, GL_COMPILE_STATUS, &compiled );

	if ( !compiled ) {
		GLsizei infoLogLength = 0;
		GLchar infoLog[1024] = {};
		glGetShaderInfoLog( id, sizeof( infoLog ), &infoLogLength, infoLog );
		CYB_ERROR( "Failed to compile shader (", id, "):\n", string, "\n",
					"--------------------------------------------------------\n",
					infoLog );
		glDeleteShader( id );
		id = 0;
	}
}

void ShaderGL::Destroy() {
	if ( id != 0 ) {
		glDeleteShader( id );
		id = 0;
	}
}

UniformGL::Enum GetPredefinedUniformEnum( const char *uniform ) {
	for ( uint16_t i = 0; i < UniformGL::Count; i++ ) {
		if ( strcmp( s_predefinedUniformNames[i], uniform ) == 0 ) {
			return static_cast<UniformGL::Enum>( i );
		}
	}

	return UniformGL::Count;
}

ProgramGL::ProgramGL() :
	id( 0 ),
	numPredefined( 0 ) {
	memset( attributes, 0xff, sizeof( attributes ) );
}

void ProgramGL::Create( const ShaderGL &vertexShader, const ShaderGL &fragmentShader ) {
	id = glCreateProgram();

	if ( vertexShader.id != 0 ) {
		glAttachShader( id, vertexShader.id );
	}

	if ( fragmentShader.id != 0 ) {
		glAttachShader( id, fragmentShader.id );
	}

	glLinkProgram( id );

	if ( vertexShader.id != 0 ) {
		glDetachShader( id, vertexShader.id );
	}

	if ( fragmentShader.id != 0 ) {
		glDetachShader( id, fragmentShader.id );
	}

	GLint linked = 0;
	glGetProgramiv( id, GL_LINK_STATUS, &linked );

	if ( !linked ) {
		GLsizei infoLogLength = 0;
		GLchar infoLog[1024] = {};
		glGetProgramInfoLog( id, sizeof( infoLog ), &infoLogLength, infoLog );
		CYB_ERROR( "Failed to link program (", vertexShader.id, ",", fragmentShader.id, "):\n", infoLog );

		glDeleteProgram( id );
		id = 0;
		return;
	}

	CYB_DEBUG( "Program (", id, ") info:" );

	// Allocate a string capable of storing max length variable names
	GLint max0, max1;
	glGetProgramInterfaceiv( id, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &max0 );
	glGetProgramInterfaceiv( id, GL_UNIFORM,       GL_MAX_NAME_LENGTH, &max1 );
	uint32_t maxLength = std::max<GLint>( max0, max1 );
	auto name = std::make_unique<char[]>( maxLength + 1 );

	struct ResourceInfoGL {
		GLenum type;
		GLint  location;
		GLint  num;
	};
	ResourceInfoGL resourceInfo;
	const GLenum props[] = { GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
	const GLsizei propCount = sizeof( props ) / sizeof( props[0] );

	// Load attributes
	GLint attribCount = 0;
	glGetProgramInterfaceiv( id, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &attribCount );
	while ( attribCount-- ) {
		glGetProgramResourceiv( id, GL_PROGRAM_INPUT, attribCount, propCount, props, propCount, NULL, (GLint *) &resourceInfo );
		glGetProgramResourceName( id, GL_PROGRAM_INPUT, attribCount, maxLength, NULL, &name[0] );

		if ( resourceInfo.location != -1 ) {
			const VertexAttribute::Enum attributeEnum = GetVertexAttributeEnum( name.get() );
			attributes[attributeEnum] = resourceInfo.location;
			CYB_DEBUG( "  layout( location = ", resourceInfo.location, " ) in ", GLEnumToString( resourceInfo.type ), " ", name.get() );
		}
	}

	// Load uniforms
	GLint uniformCount = 0;
	glGetProgramInterfaceiv( id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount );
	while ( uniformCount-- ) {
		glGetProgramResourceiv( id, GL_UNIFORM, uniformCount, propCount, props, propCount, NULL, (GLint *) &resourceInfo );
		glGetProgramResourceName( id, GL_UNIFORM, uniformCount, maxLength, NULL, &name[0] );

		const UniformGL::Enum uniformEnum = GetPredefinedUniformEnum( name.get() );
		if ( uniformEnum != UniformGL::Count ) {
			UniformGL &uniform = predefined[numPredefined];
			uniform.type = uniformEnum;		// mapped to predefined enum type
			uniform.location = resourceInfo.location;
			uniform.num = resourceInfo.num;
			numPredefined++;
		} else {
			// TODO: Cache non-predefined uniforms
		}

		CYB_DEBUG( "  layout( location = ", resourceInfo.location, " ) uniform ", GLEnumToString( resourceInfo.type ), " ", name.get() );
	}
}

void ProgramGL::Destroy() {
	if ( id != 0 ) {
		glDeleteProgram( id );
		id = 0;
	}
}

void ProgramGL::BindAttributes( const VertexLayout &layout ) {
	for ( const auto &entry : layout.entries ) {
		const GLint location = attributes[entry.attribute];

		if ( location != -1 ) {
			if ( entry.attribute != VertexAttribute::Count ) {
				const uintptr_t offset = layout.offset[entry.attribute];
				const GLint location = attributes[entry.attribute];

				glEnableVertexAttribArray( location );
				glVertexAttribPointer( location, entry.num, s_attributeType[entry.type], entry.normalized, layout.stride, (void *) offset );
			} else {
				glDisableVertexAttribArray( location );
			}
		}
	}
}

struct UniformGLUpdater {
	UniformGLUpdater( const glm::mat4 &view, const glm::mat4 &proj ) :
		m_view( view ),
		m_proj( proj ) {
		m_viewProj = m_proj * m_view;
		m_invViewCached = false;
		m_invProjCached = false;
		m_invViewProjCached = false;
	}

	void UpdatePredefined( const ProgramGL &program, const glm::mat4 &model ) {
		for ( uint16_t i = 0; i < program.numPredefined; i++ ) {
			const UniformGL &uniform = program.predefined[i];

			switch ( uniform.type ) {
			case UniformGL::View:
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_view ) );
				break;

			case UniformGL::InvView:
				if ( !m_invViewCached ) {
					m_invView = glm::inverse( m_view );
					m_invViewCached = true;
				}

				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_invView ) );
				break;

			case UniformGL::Proj:
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_proj ) );
				break;

			case UniformGL::InvProj:
				if ( !m_invProjCached ) {
					m_invView = glm::inverse( m_proj );
					m_invProjCached = true;
				}

				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_invProj ) );
				break;

			case UniformGL::ViewProj:
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_viewProj ) );
				break;

			case UniformGL::InvViewProj:
				if ( !m_invViewProjCached ) {
					m_invViewProj = glm::inverse( m_viewProj );
					m_invViewProjCached = true;
				}

				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( m_invViewProj ) );
				break;

			case UniformGL::Model:
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( model ) );
				break;

			case UniformGL::ModelView: {
				const glm::mat4 modelView = m_view * model;
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( modelView ) );
			} break;

			case UniformGL::ModelViewProj: {
				const glm::mat4 modelViewProj = m_viewProj * model;
				glUniformMatrix4fv( uniform.location, 1, GL_FALSE, glm::value_ptr( modelViewProj ) );
			} break;

			default: CYB_CHECK( false, "Unknown uniform" );
			}
		}
	}

	const glm::mat4 &m_view;
	glm::mat4 m_invView;
	const glm::mat4 &m_proj;
	glm::mat4 m_invProj;
	glm::mat4 m_viewProj;
	glm::mat4 m_invViewProj;

	bool m_invViewCached;
	bool m_invProjCached;
	bool m_invViewProjCached;
};

void RendererDriverGL::Init() {
	CYB_INFO( "Using OpenGL version ", glGetString( GL_VERSION ) );
	CYB_INFO( "Shader language ", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	glewExperimental = true;
	GLenum err = glewInit();
	CYB_FATAL( err == GLEW_OK, "Failed to initialize GLEW, ", glewGetErrorString( err ) );
	CYB_INFO( "GLEW ", glewGetString( GLEW_VERSION ) );

	/*
	GLint numExtensions = 0;
	glGetIntegerv( GL_NUM_EXTENSIONS, &numExtensions );
	CYB_DEBUG( "Found ", numExtensions, " extensions:" );
	for ( GLint i = 0; i < numExtensions; i++ ) {
		const char *extension = (const char *) glGetStringi( GL_EXTENSIONS, i );
		CYB_DEBUG( "  ", i, ") ", extension );
	}
	*/

	bool missingRequiredExtensions = false;
	for ( int i = 0; i < ExtensionGL::Count; i++ ) {
		s_extension[i].supported = glewIsSupported( s_extension[i].extension ) != 0;

		std::stringstream stream;
		stream << std::setw( 32 ) << std::left << s_extension[i].extension;
		if ( !s_extension[i].supported ) {
			missingRequiredExtensions = s_extension[i].required ? true : missingRequiredExtensions;
			stream << " [Missing]" << (s_extension[i].required ? " (required)" : "");
		} else {
			stream << " [Supported]";
		}

		CYB_DEBUG( stream.str() );
	}

	CYB_FATAL( !missingRequiredExtensions, "Required extensions(s) is missing fron OpenGL driver." );

	if ( s_extension[ExtensionGL::ARB_debug_output].supported ) {
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
		glDebugMessageCallbackARB( &DebugOutputProc, NULL );
		//glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE );
	}

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
}

void RendererDriverGL::Shutdown() {
	glDeleteVertexArrays( 1, &m_vao );
}

void RendererDriverGL::CreateVertexBuffer( const VertexBufferHandle handle, const MemoryPtr mem, const VertexLayoutHandle layoutHandle ) {
	m_vertexBuffers[handle.index].Create( mem, layoutHandle );
}

void RendererDriverGL::DestroyVertexBuffer( const VertexBufferHandle handle ) {
	m_vertexBuffers[handle.index].Destroy();
}

void RendererDriverGL::CreateIndexBuffer( const IndexBufferHandle handle, const MemoryPtr mem ) {
	m_indexBuffers[handle.index].Create( mem );
}

void RendererDriverGL::DestroyIndexBuffer( const IndexBufferHandle handle ) {
	m_indexBuffers[handle.index].Destroy();
}

void RendererDriverGL::CreateVertexLayout( const VertexLayoutHandle handle, const VertexLayout &layout ) {
	memcpy( &m_vertexLayouts[handle.index], &layout, sizeof( VertexLayout ) );
}

void RendererDriverGL::DestroyVertexLayout( const VertexLayoutHandle handle ) {
	m_vertexLayouts[handle.index].Reset();
}

void RendererDriverGL::CreateProgram( const ShaderProgramHandle handle, const MemoryPtr vertexShaderMem, const MemoryPtr fragmentShaderMem ) {
	ShaderGL vertexShader = {};
	ShaderGL fragmentShader = {};

	vertexShader.Create( vertexShaderMem, GL_VERTEX_SHADER );
	fragmentShader.Create( fragmentShaderMem, GL_FRAGMENT_SHADER );
	m_shaderPrograms[handle.index].Create( vertexShader, fragmentShader );
	vertexShader.Destroy();
	fragmentShader.Destroy();
}

void RendererDriverGL::DestroyProgram( const ShaderProgramHandle handle ) {
	m_shaderPrograms[handle.index].Destroy();
}

static void ClearBuffers( const CommandBuffer *cbuf ) {
	const uint32_t flags = cbuf->m_clearFlags;
	GLbitfield clearBitFlags = 0;

	if ( flags & ClearFlags::Color ) {
		const float *rgba = cbuf->m_clearColor;
		glClearColor( rgba[0], rgba[1], rgba[2], rgba[3] );
		clearBitFlags |= GL_COLOR_BUFFER_BIT;
	}

	if ( flags & ClearFlags::Depth ) {
		glClearDepth( cbuf->m_clearDepth );
		clearBitFlags |= GL_DEPTH_BUFFER_BIT;
	}

	if ( flags & ClearFlags::Stencil ) {
		glClearStencil( cbuf->m_clearStencil );
		clearBitFlags |= GL_STENCIL_BUFFER_BIT;
	}

	if ( flags != 0 ) {
		glClear( clearBitFlags );
	}
}

void RendererDriverGL::Commit( const CommandBuffer *cbuf ) {
	ClearBuffers( cbuf );

	glBindVertexArray( m_vao );

	UniformGLUpdater uniformUpdater( cbuf->m_viewMatrix, cbuf->m_projMatrix );
	DrawCommand currentState;
	currentState.Clear();

	for ( const DrawCommand *draw = cbuf->DrawCommands().Next(); draw != nullptr; draw = draw->listNode.Next() ) {
		bool programChanged = false;

		// Update shader program state
		if ( currentState.shaderProgram.index != draw->shaderProgram.index ) {
			currentState.shaderProgram = draw->shaderProgram;
			GLuint id = IsValid( draw->shaderProgram ) ? m_shaderPrograms[draw->shaderProgram.index].id : 0;
			glUseProgram( id );
			programChanged = true;
		}

		// Update geometry buffers state
		if ( programChanged ||
			currentState.vertexBuffer.index != draw->vertexBuffer.index ||
			currentState.indexBuffer.index != draw->indexBuffer.index ) {

			currentState.vertexBuffer = draw->vertexBuffer;
			currentState.indexBuffer = draw->indexBuffer;

			if ( IsValid( draw->vertexBuffer ) ) {
				const VertexBufferGL &vertexBuffer = m_vertexBuffers[draw->vertexBuffer.index];
				glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer.id );
				m_shaderPrograms[draw->shaderProgram.index].BindAttributes( m_vertexLayouts[vertexBuffer.vertexLayout.index] );
			}

			if ( IsValid( draw->indexBuffer ) ) {
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[draw->indexBuffer.index].id );
			}
		}

		// Render the current state
		if ( IsValid( currentState.shaderProgram ) ) {
			if ( IsValid( currentState.vertexBuffer ) ) {
				uniformUpdater.UpdatePredefined( m_shaderPrograms[currentState.shaderProgram.index], draw->transform );

				if ( IsValid( currentState.indexBuffer ) ) {
					index_t numIndices = draw->numIndices;
					if ( numIndices == INDEX_MAX ) {
						numIndices = m_indexBuffers[currentState.indexBuffer.index].size / sizeof( index_t );
					}

					glDrawElementsInstanced( GL_TRIANGLES, numIndices, INDEX_TYPE_GL, (void *) 0, 1 );
				}
			}
		}
	}
}

}	// namespace cyb