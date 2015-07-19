set( RENDERER_SOURCES
	command.cpp			command.h
	driver_gl.cpp				driver_gl.h
	frontend.cpp				frontend.h
	renderer.cpp				renderer.h
	vertex_layout.cpp		vertex_layout.h
)

if( GLEW_SOURCE )
	message( STATUS "Found GLEW source code. Linking statically" )
	list( APPEND RENDERER_SOURCES ${GLEW_SOURCE} )
else()
	list( APPEND RENDERER_LIBS ${GLEW_LIBRARY} )
endif()


add_library( renderer  STATIC ${RENDERER_SOURCES} )
target_link_libraries( renderer )