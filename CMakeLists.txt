cmake_minimum_required(VERSION 2.8)

project(tracer-analyzer)

add_definitions("-g")

# so that we can include from helpers and from one project to another
include_directories(".")

file(GLOB_RECURSE FILTER_SRC "filter/*.cpp" "filter/*.h")

file(GLOB_RECURSE HMARRAYS_SRC "hmarrays/*.cpp" "hmarrays/*.h")

add_executable(filter ${FILTER_SRC})
add_executable(hmarrays ${HMARRAYS_SRC})
