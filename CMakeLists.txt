cmake_minimum_required(VERSION 2.8)

project(tracer-analyzer)

# so that we can include from helpers and from one project to another
include_directories(".")

file(GLOB_RECURSE FILTER_SRC "filter/*.cpp" "filter/*.h")

add_executable(filter ${FILTER_SRC})
