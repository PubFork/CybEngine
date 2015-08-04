message( STATUS "Processing Renderer..." )

# Setup source groups
file( GLOB RENDERER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h )
file( GLOB RENDERER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

if( GLEW_SOURCE )
	message( STATUS "Found GLEW source code. Linking statically" )
	list( APPEND RENDERER_SOURCES ${GLEW_SOURCE} )
else()
	list( APPEND RENDERER_LIBS ${GLEW_LIBRARY} )
endif()

source_group( "Header Files" FILES ${PRIVATE_RENDERER_HEADERS} )
source_group( "Source Files" FILES ${PRIVATE_RENDERER_SOURCES} )

# Setup static library
set( RENDERER_FILES ${RENDERER_HEADERS} ${RENDERER_SOURCES} )
add_library( renderer STATIC ${RENDERER_FILES} )
target_link_libraries( renderer )