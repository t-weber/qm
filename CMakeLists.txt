#
# @author Tobias Weber (orcid: 0000-0002-7230-1932)
# @date Oct-2020
# @license see 'LICENSE' file
#

project(qm)
cmake_minimum_required(VERSION 3.0)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-std=c++20)
add_compile_options(-Wall -Wextra -Weffc++)

find_package(Boost REQUIRED)
add_compile_options(${Boost_CXX_FLAGS})

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)


include_directories(
	${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src
	${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}/..
	${Qt5Svg_INCLUDE_DIRS}
)


add_executable(qm
	src/gui/main.cpp
	src/gui/qm_gui.cpp src/gui/qm_gui.h
	src/gui/workspace.cpp src/gui/workspace.h
	src/gui/components.cpp src/gui/components.h
	src/gui/components_table.cpp src/gui/components_table.h
	src/gui/component_properties.cpp src/gui/component_properties.h
	src/gui/component_operator.cpp src/gui/component_operator.h
	src/gui/settings.cpp src/gui/settings.h
	src/gui/globals.cpp src/gui/globals.h
	src/gui/about.cpp src/gui/about.h
	src/gui/resources.cpp src/gui/resources.h
	src/gui/recent.h src/gui/types.h src/gui/helpers.h
	src/lib/math_algos.h src/lib/math_conts.h src/lib/math_concepts.h
	src/lib/qm_algos.h
)

target_link_libraries(qm
	Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg
	${Boost_LIBRARIES}
)
