CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(DrawingAssociation)


#HEADER FILE
set(draAssociation_hdrs
	src/coordinateSystem.h
	src/dataIntegration.h
	src/columnInfo.h
	src/beamInfo.h
	src/drawing.h
	src/columnProcessInfo.h
	src/RWJsonFile.h
	src/wallInfo.h
	src/wallProcessInfo.h
	src/axisProcessInfo.h
)

#SOURCE FILE
set(draAssociation_srcs
	src/coordinateSystem.cpp
	src/dataIntegration.cpp
	src/columnInfo.cpp
	src/beamInfo.cpp
	src/drawing.cpp
	src/columnProcessInfo.cpp
	src/RWJsonFile.cpp
	src/wallInfo.cpp
	src/wallProcessInfo.cpp
	src/axisProcessInfo.cpp
)



# request for c++11 or C++14
if(CMAKE_HOST_WIN32)
add_compile_options(/std:c++14)
# disable unsafe notification
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_HOST_UNIX)
add_compile_options(-std=c++11)
add_definitions(-g)
endif()

find_package(jsoncpp CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)

# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(drawingAssociation STATIC ${draAssociation_hdrs} ${draAssociation_srcs})
target_link_libraries(drawingAssociation PRIVATE spdlog::spdlog jsoncpp_lib cereal )

