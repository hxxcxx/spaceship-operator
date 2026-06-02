cmake_minimum_required(VERSION 3.20)
project(spaceship-operator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8 /W4 /permissive- /Zc:__cplusplus)
endif()

add_subdirectory(src/01_comparison)
add_subdirectory(src/02_placeholder_types)
