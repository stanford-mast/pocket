PROJECT(rpc)
cmake_minimum_required(VERSION 2.6)
add_subdirectory(client)
include_directories ("${PROJECT_SOURCE_DIR}/client")
add_subdirectory(pocket)
include_directories ("${PROJECT_SOURCE_DIR}/pocket")
#add_subdirectory(shell)
add_subdirectory(iobench)

