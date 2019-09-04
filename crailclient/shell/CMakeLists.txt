set(CMAKE_CXX_FLAGS "-std=c++11 -g")
find_package( PythonLibs 2.7 REQUIRED )
find_package( PythonInterp 2.7 REQUIRED )
include_directories(${PYTHON_LIBRARIES})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})


add_executable(pshell pshell.cc)
target_link_libraries (pshell LINK_PUBLIC pocket ${PYTHON_LIBRARIES})
install(TARGETS pshell DESTINATION /bin)
