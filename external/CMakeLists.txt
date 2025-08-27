# External dependencies setup

add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(glm)

# Add Perlin Noise as INTERFACE
add_library(perlin INTERFACE)
target_include_directories(perlin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/perlin)
