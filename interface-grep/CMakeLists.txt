project(interface-grep)
cmake_minimum_required(VERSION 3.12)
set( EXECUTABLE_OUTPUT_PATH ../../bin)

add_executable(interface-grep main.cc)
target_link_libraries(interface-grep -lwin32hlp -lexshit 
	-lstdshit -lcomdlg32 -limagehlp -lgdi32)
