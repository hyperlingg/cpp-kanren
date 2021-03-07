cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
project(cpp-kanren VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_INSTALL_RPATH "/usr/local/lib64/")


add_definitions( "-fcoroutines -fno-exceptions" )
add_subdirectory(include)

file(GLOB SOURCES "*.cpp")

add_executable(cpp-kanren ${SOURCES})

# target_link_libraries(cpp-kanren PUBLIC ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)