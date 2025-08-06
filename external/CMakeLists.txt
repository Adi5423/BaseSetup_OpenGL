#cmake_minimum_required(VERSION 3.15)
#project(MineCraft_Clone VERSION 0.1 LANGUAGES CXX)

# Set output directories
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories


add_subdirectory(glfw)

add_subdirectory(glad) # <-- Use CMakeLists.txt inside glad

#add_subdirectory(stb)


add_subdirectory(glm)