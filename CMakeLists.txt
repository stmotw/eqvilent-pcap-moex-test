project(eqvilent)
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall")

set(outdir ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${outdir} CACHE PATH "Build directory" FORCE)

file(GLOB ${PROJECT_NAME}_SRC
    "example/*.cpp"
    "include/eqvilent/*.h"
)
include_directories(
    "include/"
)

add_executable(
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
)