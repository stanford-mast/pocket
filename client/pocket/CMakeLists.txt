set(CMAKE_CXX_FLAGS "-fPIC -std=c++14 -g")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})


FIND_PACKAGE(Boost 1.58.0)
IF(Boost_FOUND)
	INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}" "/usr/include/python3.5")
	SET(Boost_USE_STATIC_LIBS OFF)
	SET(Boost_USE_MULTITHREADED ON)
	SET(Boost_USE_STATIC_RUNTIME OFF)
	FIND_PACKAGE(Boost 1.58.0 COMPONENTS python)

	ADD_LIBRARY(pocket SHARED pypocket.cxx pocket_dispatcher.cc)
	TARGET_LINK_LIBRARIES(pocket ${Boost_LIBRARIES} cppcrail)
ELSEIF(NOT Boost_FOUND)
	MESSAGE(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
ENDIF()

install(TARGETS pocket DESTINATION /lib)
install (FILES 
	pocket_dispatcher.h
	DESTINATION /include)

