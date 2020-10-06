project(interface-view)
cmake_minimum_required(VERSION 3.12)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_link_options("-mwindows")
endif()

add_executable(interface-view
	resource.rc
	main.cc
	interface_data.cc
	selfont.cc
	util.cc
)


target_link_libraries(interface-view -lwin32hlp -lexshit
	-lstdshit)
