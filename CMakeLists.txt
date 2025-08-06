cmake_minimum_required(VERSION 3.15)
project(Minecraft_Clone VERSION 0.1 LANGUAGES CXX)

# Force static runtime for all MSVC targets , for static linking.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(external)
add_subdirectory(src)

