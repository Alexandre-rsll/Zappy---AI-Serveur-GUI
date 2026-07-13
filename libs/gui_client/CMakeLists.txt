cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gui_client)

add_library(${PROJECT_NAME} STATIC)
add_subdirectory(src)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_RAYLIB_CPP_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_Declare(
        raylib_cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG v5.5.0
)

FetchContent_MakeAvailable(raylib raylib_cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        ZAPPY_ASSETS_DIR="${CMAKE_SOURCE_DIR}/gui/assets"
)

target_link_libraries(${PROJECT_NAME} PUBLIC logger cli gui_protocol raylib raylib_cpp)
