add_executable(Minecraft_Clone
    main.cpp
    Camera.cpp
    Chunk.cpp
)

target_include_directories(Minecraft_Clone PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
    ${PROJECT_SOURCE_DIR}/external/stb
    ${PROJECT_SOURCE_DIR}/external/glm
    ${PROJECT_SOURCE_DIR}/external/perlin
)

target_link_libraries(Minecraft_Clone PRIVATE glfw glad perlin)
