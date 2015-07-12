set( CORE_SOURCES
							config.h
							handles.h
	logger.cpp			logger.h
	memory.cpp		memory.h
							interface.h
)

add_library( core  STATIC ${CORE_SOURCES} )
target_link_libraries( core )