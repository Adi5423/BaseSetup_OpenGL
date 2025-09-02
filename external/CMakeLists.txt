# external/CMakeLists.txt - add third party subdirs here
add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(glm)

# Perlin header-only interface library
add_library(perlin INTERFACE)
target_include_directories(perlin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/perlin)

# stb is header-only (no add_subdirectory needed)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)
