cmake_minimum_required(VERSION 3.15)
project(Minecraft_Clone VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static runtime for MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

# Output binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# External libraries
add_subdirectory(external)

# Engine static lib
add_subdirectory(engine)

# Add game (executable)
add_subdirectory(app/game)   # <-- THIS ensures your game builds