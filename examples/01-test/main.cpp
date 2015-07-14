#include <cstdio>	// snprintf
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/interface.h"
#include "core/logger.h"
#include "core/memory.h"
#include "utils/timer.h"
#include "renderer/renderer.h"
#include "renderer/driver_gl.h"

static const char *s_vertexShaderStr = "\
#version 330 core \n\
in vec3 a_position; \n\
in vec3 a_color0; \n\
out vec3 fragmentColor; \n\
uniform mat4 u_modelViewProj; \n\
void main() {\n\
	gl_Position = u_modelViewProj * vec4( a_position, 1 ); \n\
	fragmentColor = a_color0; \n\
}";

static const char *s_fragmentShaderStr = "\
#version 330 core \n\
in vec3 fragmentColor; \n\
out vec3 color; \n\
void main() { \n\
	color = fragmentColor; \n\
}";

struct PosColorVertex {
	float x;
	float y;
	float z;
	uint32_t abgr;

	static const cyb::VertexLayout s_layout;
};

const cyb::VertexLayout PosColorVertex::s_layout = {
	{ cyb::VertexAttribute::Position, cyb::AttributeType::Float, 3, false },
	{ cyb::VertexAttribute::Color0,   cyb::AttributeType::Uint8, 4, true  }
};

static const PosColorVertex s_cubeVertices[] = {
	{ -1.0f,  1.0f,  1.0f, 0xff000000 },
	{  1.0f,  1.0f,  1.0f, 0xff0000ff },
	{ -1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{  1.0f, -1.0f,  1.0f, 0xff00ffff },
	{ -1.0f,  1.0f, -1.0f, 0xffff0000 },
	{  1.0f,  1.0f, -1.0f, 0xffff00ff },
	{ -1.0f, -1.0f, -1.0f, 0xffffff00 },
	{  1.0f, -1.0f, -1.0f, 0xffffffff }
};

static const unsigned short s_cubeIndices[] = {
	0, 1, 2, 1, 3, 2,
	4, 6, 5, 5, 6, 7,
	0, 2, 4, 4, 2, 6,
	1, 5, 3, 5, 7, 3,
	0, 4, 1, 4, 5, 1,
	2, 3, 6, 6, 3, 7
};

int main() {
	cyb::g_logger->AddPolicy( std::make_unique<cyb::LogPolicy_Stdout>( nullptr ), cyb::LogPolicyOperation::CopyHistory );

	//=========== Inititalize glfw & glew
	if ( !glfwInit() ) {
		return 1;
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 4 );
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, 1 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	GLFWwindow *window = glfwCreateWindow( 1024, 760, "cyb: 01-test", NULL, NULL );
	if ( !window ) {
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent( window );
	glfwSwapInterval( 0 );
	
	// Create and initialize renderer driver
	auto renderer = std::make_unique<cyb::Renderer>();
	renderer->Init();
	cyb::CommandBuffer *cbuf = renderer->CreateCommandBuffer( CYB_MEGABYTES( 8 ) );
	cbuf->SetClearColor( 0.56f, 0.56f, 0.63f, 1.0f );

	//=========== Load shaders
	cyb::ShaderProgramHandle programHandle = renderer->CreateProgram( cyb::MemMakeRef( s_vertexShaderStr,   strlen( s_vertexShaderStr ) ),
		                                                              cyb::MemMakeRef( s_fragmentShaderStr, strlen( s_fragmentShaderStr ) ) );

	//=========== Load geometry
	cyb::VertexBufferHandle cubeVerticesHandle = renderer->CreateVertexBuffer( cyb::MemMakeRef( s_cubeVertices, sizeof( s_cubeVertices ) ), PosColorVertex::s_layout );
	cyb::IndexBufferHandle cubeIndexHandle = renderer->CreateIndexBuffer( cyb::MemMakeRef( s_cubeIndices, sizeof( s_cubeIndices ) ) );

	//=========== Create transformation matrices
	glm::mat4 projection = glm::perspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );
	glm::mat4 view = glm::lookAt(
		glm::vec3( 0, 0, -30 ),		// position
		glm::vec3( 0, 0, 0 ),		// target
		glm::vec3( 0, 1, 0 ) );		// up
	renderer->UpdateViewTransform( &view, &projection );

	//=========== Main loop
	cyb::FrameTimer timer;
	char titleBuffer[64];
	while ( !glfwWindowShouldClose( window ) ) {
		double frameTime = timer.Frame();

		_snprintf( titleBuffer, 64, "FrameTime: %.0fms", (float) timer.GetFrameTimeMs() );
		glfwSetWindowTitle( window, titleBuffer );

#define CUBE_NUM_HORIZONTAL (40*1.0)
#define CUBE_NUM_VERTICAL	(32*1.0)
		for ( uint16_t x = 0; x < CUBE_NUM_HORIZONTAL; x++ ) {
			for ( uint16_t y = 0; y < CUBE_NUM_VERTICAL; y++ ) {
				glm::mat4 rotate = glm::rotate( glm::mat4( 1.0f ), (float) ( frameTime + x*0.21 ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
				rotate = glm::rotate( glm::mat4( rotate ), (float) ( frameTime + y*0.37 ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

				float xpos = float( x ) / float( CUBE_NUM_HORIZONTAL - 1 ) * 2.0f - 1.0f;
				float ypos = float( y ) / float( CUBE_NUM_VERTICAL   - 1 ) * 2.0f - 1.0f;
				glm::mat4 translate = glm::translate( glm::mat4( 1.0f ), glm::vec3(
					xpos * 20.0f,
					ypos * 14.0f,
					2.3f * sin( frameTime + ( x + y )*0.42f ) ) );

				cyb::DrawCommand *draw = cbuf->AddDrawCommand();
				draw->vertexBuffer = cubeVerticesHandle;
				draw->indexBuffer = cubeIndexHandle;
				draw->shaderProgram = programHandle;
				draw->transform = translate * rotate;
			}
		}

		//renderer->Submit( cbuf );
		renderer->Frame();

		glfwSwapBuffers( window );
		glfwPollEvents();
	}

	renderer->Shutdown();
	glfwTerminate();
	return 0;
}