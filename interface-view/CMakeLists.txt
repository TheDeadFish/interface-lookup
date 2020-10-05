project(interface-view)
cmake_minimum_required(VERSION 3.12)
#add_link_options(-s -Wl,-gc-sections -mwindows)

add_executable(interface-view
	resource.rc
	main.cc
	interface_data.cc
)


target_link_libraries(interface-view -lwin32hlp -lexshit
	-lstdshit)
