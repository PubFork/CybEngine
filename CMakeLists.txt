cmake_minimum_required( VERSION 2.8 )
project( cyb )

set( CMAKE_MODULE_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )

set( GLEW_LOCATION ${CMAKE_SOURCE_DIR}/../LIBS/glew-1.12.0 )
set( GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../LIBS/glm )
find_package( OpenGL )
find_package( GLEW REQUIRED )
if( GLEW_SOURCE )
	add_definitions( -DGLEW_STATIC )
endif()
find_package( GLM REQUIRED )

include_directories(	
	${PROJECT_SOURCE_DIR}
	${GLEW_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
)

if( MSVC )
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

# Add glfw source to the solution
add_subdirectory( extern/glfw-3.1.1 )
include_directories(  extern/glfw-3.1.1/include )

add_subdirectory( core )
add_subdirectory( renderer )
add_subdirectory( utils )
add_subdirectory( examples )

# Add asset files to the solution
file( GLOB_RECURSE ASSET_FILES ${CMAKE_SOURCE_DIR}/examples/assets/* )
add_custom_target(
	assets ALL
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/example/assets ${PROJECT_BINARY_DIR}/assets
	SOURCES ${RESOURCE_FILES}
)