# external/CMakeLists.txt - add third party subdirs here
add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(glm)

# Perlin header-only interface library (make sure external/perlin/PerlinNoise.hpp exists)
add_library(perlin INTERFACE)
target_include_directories(perlin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/perlin)
