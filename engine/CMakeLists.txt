# engine/CMakeLists.txt
add_library(engine STATIC
    core/src/Camera.cpp
    core/src/Collision.cpp
    world/src/Chunk.cpp

    "core/src/Physics.cpp"

    # Physics is still in top-level src (not yet moved into engine)
   # ${PROJECT_SOURCE_DIR}/src/Physics.cpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/world/include
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/include
    ${PROJECT_SOURCE_DIR}/include                # top-level include
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
    ${PROJECT_SOURCE_DIR}/external/glm
    ${PROJECT_SOURCE_DIR}/external/perlin
    ${PROJECT_SOURCE_DIR}/external/stb
)
