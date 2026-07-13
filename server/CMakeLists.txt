cmake_minimum_required(VERSION 3.20)

project(server)

add_executable(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)

target_include_directories(${PROJECT_NAME} PRIVATE include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE network_server cli)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX "zappy_"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
