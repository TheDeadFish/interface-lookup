project(interface-grep)
cmake_minimum_required(VERSION 3.12)

add_executable(interface-grep main.cc)
target_link_libraries(interface-grep -lwin32hlp -lexshit 
	-lstdshit -lcomdlg32 -limagehlp -lgdi32)
