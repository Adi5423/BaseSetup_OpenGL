# src/CMakeLists.txt
add_executable(Minecraft_Clone main.cpp)
target_include_directories(Minecraft_Clone PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/glfw/include
  ${PROJECT_SOURCE_DIR}/external/glad/include
  ${PROJECT_SOURCE_DIR}/external/stb
  ${PROJECT_SOURCE_DIR}/external/glm
)
target_link_libraries(Minecraft_Clone PRIVATE glfw glad)  # stb/glm header-only
