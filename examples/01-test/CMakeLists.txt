add_executable( 01-test
	main.cpp
)

target_link_libraries( 01-test core renderer utils glfw ${GLFW_LIBRARIES} )