CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
PROJECT (JSONCPP)

# SOURCE FILES
set(jsoncpp_srcs
    overrides/src/lib_json/json_reader.cpp
    overrides/src/lib_json/json_value.cpp
    source/src/lib_json/json_writer.cpp
)

# HEADER FILES
set(jsoncpp_hdrs
    source/include/json/assertions.h
    source/include/json/autolink.h
    source/include/json/config.h
    source/include/json/features.h
    source/include/json/forwards.h
    source/include/json/json.h
    source/include/json/reader.h
    overrides/include/json/value.h
    source/include/json/writer.h
    source/src/lib_json/json_batchallocator.h
    source/src/lib_json/json_tool.h
)

# request for c++11
if(CMAKE_HOST_WIN32)
add_compile_options(/std:c++14)
# disable unsafe notification
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_HOST_UNIX)
add_compile_options(-std=c++11)
add_definitions(-g)
endif()

add_definitions(-DJSON_USE_EXCEPTION=0)

include_directories(
    overrides/include/
    source/include/
    source/src/lib_json/
)
  
# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(jsoncpp STATIC ${jsoncpp_srcs} ${jsoncpp_hdrs})
