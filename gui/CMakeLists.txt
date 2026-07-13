cmake_minimum_required(VERSION 3.20)

project(gui)

add_executable(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)

include_directories(include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE gui_client)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX "zappy_"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
