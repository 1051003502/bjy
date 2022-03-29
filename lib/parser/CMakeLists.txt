CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(Parser)


#interface
set(INTERFACE
	interface/parser.h
	interface/parser.cpp
	interface/transer.h
	interface/transer.cpp
	interface/RTree.h
	interface/headerFile.h
)

set(BASE
	utils/base/axis.h
	utils/base/axis.cpp
	utils/base/axisSet.h
	utils/base/axisSet.cpp
	utils/base/block.h
	utils/base/block.cpp
	utils/base/blockSet.h
	utils/base/blockSet.cpp
	utils/base/border.h
	utils/base/border.cpp
	utils/base/borderSet.h
	utils/base/borderSet.cpp
	utils/base/box.h
	utils/base/box.cpp
	utils/base/circle.h
	utils/base/circle.cpp
	utils/base/common.h
	utils/base/common.cpp
	utils/base/component.h
	utils/base/component.cpp
	utils/base/corner.h
	utils/base/corner.cpp
	utils/base/dimension.h
	utils/base/dimension.cpp
	utils/base/entry.h
	utils/base/entry.cpp
	utils/base/kdt.h
	utils/base/line.h
	utils/base/line.cpp
	utils/base/log.h
	utils/base/log.cpp
	utils/base/point.h
	utils/base/point.cpp
	utils/base/signal.h
	utils/base/table.h
	utils/base/table.cpp
	utils/base/transform.h
	utils/base/transform.cpp
	utils/base/sectionSymbolSet.h
	utils/base/sectionSymbolSet.cpp
	utils/base/sectionSymbol.h
	utils/base/sectionSymbol.cpp
)


set(BEAM
	utils/beam/beam.h
	utils/beam/beam.cpp
	utils/beam/beamSet.h
	utils/beam/beamSet.cpp
	utils/beam/beamLongitudinal.h
	utils/beam/beamLongitudinal.cpp
	utils/beam/beamLongitudinalSet.h
	utils/beam/beamLongitudinalSet.cpp
	utils/beam/beamSection.h
	utils/beam/beamSection.cpp
	utils/beam/beamSectionSet.h
	utils/beam/beamSectionSet.cpp
)

set(COLUMN
	utils/column/pillar.h
	utils/column/pillar.cpp
	utils/column/pillarSet.h
	utils/column/pillarSet.cpp
	utils/column/ColumnSection.h
	utils/column/ColumnSection.cpp
	utils/column/ColumnSectionSet.h
	utils/column/ColumnSectionSet.cpp
)
set (FLOOR
	utils/floor/floorPlan.h
	utils/floor/floorPlan.cpp
)
set(PROFILE
	
	utils/profile/elevationSymbolSet.h
	utils/profile/elevationSymbolSet.cpp
	utils/profile/elevationSymbol.h
	utils/profile/elevationSymbol.cpp
	utils/profile/longitudinalStructProfile.h
	utils/profile/longitudinalStructProfile.cpp
	utils/profile/mainStructureSection.h
	utils/profile/mainStructureSection.cpp
	
)

set(PUBLIC
	utils/public/data.h
	utils/public/data.cpp
	utils/public/publicFunction.h
	utils/public/publicFunction.cpp
)

set(WALL
	utils/wall/wallSectionSet.h
	utils/wall/wallSectionSet.cpp
	utils/wall/wallSection.h
	utils/wall/wallSection.cpp
	utils/wall/wallLocation.h
	utils/wall/wallLocation.cpp
	utils/wall/wallLocationSet.h
	utils/wall/wallLocationSet.cpp
)



find_package(Threads)


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

source_group(interface FILES ${INTERFACE})
source_group(base FILES ${BASE})
source_group(beam FILES ${BEAM})
source_group(column FILES ${COLUMN})
source_group(floor FILES ${FLOOR})
source_group(profile FILES ${PROFILE})
source_group(public FILES ${PUBLIC})
source_group(wall FILES ${WALL})


add_library(parser STATIC ${INTERFACE} ${BASE} ${BEAM} ${COLUMN} ${FLOOR} ${PROFILE} ${PUBLIC} ${WALL})
target_link_libraries(parser PRIVATE spdlog::spdlog jsoncpp_lib cereal)

install(TARGETS parser DESTINATION lib)