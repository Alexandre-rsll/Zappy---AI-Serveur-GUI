cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(logger)

add_library(${PROJECT_NAME} STATIC)
add_subdirectory(src)

option(DBG "Debug mode" OFF)

if (DBG)
    message("DEBUG MODE ON")
    target_compile_options(${PROJECT_NAME} PRIVATE -DDEBUG)
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
