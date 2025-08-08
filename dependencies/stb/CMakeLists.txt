cmake_minimum_required(VERSION 3.1)
project(stb)

add_library(stb)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
target_sources(stb PRIVATE ${SOURCES})
target_include_directories(stb PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



