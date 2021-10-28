#
# unit tests
# @author Tobias Weber (orcid: 0000-0002-7230-1932)
# @date oct-2021
# @license see 'LICENSE' file
#

project(qm_unittests)
cmake_minimum_required(VERSION 3.0)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_compile_options(-std=c++20)
add_compile_options(-Wall -Wextra -Weffc++)

find_package(Boost REQUIRED)
add_compile_options(${Boost_CXX_FLAGS})


include_directories(
	${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/.. ${PROJECT_SOURCE_DIR}/../src
	${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}/..
)


# -----------------------------------------------------------------------------
# cnot test
# -----------------------------------------------------------------------------
add_executable(cnot cnot.cpp
	../lib/math_algos.h ../lib/math_conts.h ../lib/math_concepts.h
	../lib/qm_algos.h
)

target_link_libraries(cnot ${Boost_LIBRARIES})

add_test(NAME cnot COMMAND cnot)
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# hadamard test
# -----------------------------------------------------------------------------
add_executable(hadamard hadamard.cpp
	../lib/math_algos.h ../lib/math_conts.h ../lib/math_concepts.h
	../lib/qm_algos.h
)

target_link_libraries(hadamard ${Boost_LIBRARIES})

add_test(NAME hadamard COMMAND hadamard)
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# adder test
# -----------------------------------------------------------------------------
add_executable(adder adder.cpp
	../lib/math_algos.h ../lib/math_conts.h ../lib/math_concepts.h
	../lib/qm_algos.h
)

target_link_libraries(adder ${Boost_LIBRARIES})

add_test(NAME adder COMMAND adder)
# -----------------------------------------------------------------------------
