message( STATUS "Processing Utils..." )

# Setup source groups
file( GLOB UTILS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h )
file( GLOB UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

source_group( "Header Files" FILES ${UTILS_HEADERS} )
source_group( "Source Files" FILES ${UTILS_SOURCES} )

# Setup static library
set( UTILS_FILES ${UTILS_HEADERS} ${UTILS_SOURCES} )
add_library( utils STATIC ${UTILS_FILES} )
target_link_libraries( utils )