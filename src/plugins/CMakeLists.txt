#
# @author Tobias Weber (orcid: 0000-0002-7230-1932)
# @date Oct-2020
# @license see 'LICENSE' file
#

project(qm_plugin_example)
cmake_minimum_required(VERSION 3.0)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_VERBOSE_MAKEFILE True)
#set(CMAKE_POSITION_INDEPENDENT_CODE True)

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
	${PROJECT_SOURCE_DIR}/..
	${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}/..
	${Qt5Svg_INCLUDE_DIRS}
)


add_library(example SHARED
	example.cpp example.h
	../gui/settings.cpp ../gui/settings.h
)

set_property(TARGET example PROPERTY POSITION_INDEPENDENT_CODE True)

target_link_libraries(example
	Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg
	${Boost_LIBRARIES}
)
