set( UTILS_SOURCES	
										allocator.h
										intrusive_list.h
	timer.cpp						timer.h
)

add_library( utils  STATIC ${UTILS_SOURCES} )
target_link_libraries( utils )