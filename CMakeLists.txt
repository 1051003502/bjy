CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
# SET(CMAKE_C_COMPILER "/usr/local/bin/gcc")
# SET(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(CADAnalyser)

set(viewer_hdrs
	viewer/DrawingView.h
	viewer/resource.h
	viewer/StdAfx.h
	viewer/Viewer.h
	viewer/ViewerDlg.h
	viewer/Drawing.h
)

set(viewer_srcs
	viewer/DrawingView.cpp
	viewer/StdAfx.cpp
	viewer/Viewer.cpp
	viewer/Viewer.rc
	viewer/ViewerDlg.cpp
	viewer/Drawing.cpp
)

set(cli_srcs
	cli/main.cpp
)

include_directories(
    lib/jsoncpp/source/include/
    lib/jsoncpp/overrides/include/
    lib/spdlog/include/
    lib/libdxfrw/src/
    lib/parser/
    lib/parser/utils/
	lib/parser/interface/
	lib/parser/utils/base/
	lib/parser/utils/beam/
	lib/parser/utils/column/
	lib/parser/utils/public/
	lib/parser/utils/wall/
	lib/parser/utils/floor/
	lib/parser/utils/profile/
	lib/drawingAssociation/src/
	viewer/
)

link_directories(
    lib/jsoncpp/  
    lib/libdxfrw/
    lib/spdlog/  
    lib/parser/
	lib/drawingAssociation/
)

find_package(Threads)
add_subdirectory(lib/spdlog)
add_subdirectory(lib/libdxfrw)
add_subdirectory(lib/parser)
add_subdirectory(lib/drawingAssociation)


# request for c++11 or C++14
if(CMAKE_HOST_WIN32)
	add_compile_options(/std:c++14)
	# disable unsafe notification
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_HOST_UNIX)
	add_compile_options(-std=c++11)
	add_definitions(-g)
endif()

find_package(cereal CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_executable(CADAnalyserCLI ${cli_srcs})
target_link_libraries(CADAnalyserCLI drawingAssociation parser dxfrw spdlog::spdlog ${CMAKE_THREAD_LIBS_INIT} jsoncpp_lib cereal)


if (WIN32)
	set(CMAKE_MFC_FLAG 1)
	add_definitions(-D_AFXDLL)
	add_executable(CADAnalyser ${viewer_srcs} ${viewer_hdrs})
	set_target_properties(CADAnalyser PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	target_link_libraries(CADAnalyser drawingAssociation parser dxfrw spdlog::spdlog  jsoncpp_lib cereal )
endif (WIN32)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../deploy-service/AnalyserDwg/)


