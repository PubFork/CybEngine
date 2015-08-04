message( STATUS "Processing Core..." )

# Setup source groups
file( GLOB CORE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h )
file( GLOB CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

source_group( "Header Files" FILES ${CORE_HEADERS} )
source_group( "Source Files" FILES ${CORE_SOURCES} )

# Setup static library
set( CORE_FILES ${CORE_HEADERS} ${CORE_SOURCES} )
add_library( core STATIC ${CORE_FILES} )
target_link_libraries( core )