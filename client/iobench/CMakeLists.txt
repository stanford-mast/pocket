set(CMAKE_CXX_FLAGS "-std=c++11 -g")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
add_executable(iobench 
	iobench.cc
	)
target_link_libraries (iobench LINK_PUBLIC cppcrail)
install(TARGETS iobench DESTINATION /bin)
