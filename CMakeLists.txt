cmake_minimum_required(VERSION 3.0.0)
project(cpp-kanren VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)

file(GLOB SOURCES "*.cpp")

add_executable(cpp-kanren ${SOURCES})

# target_link_libraries(cpp-kanren PUBLIC ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)