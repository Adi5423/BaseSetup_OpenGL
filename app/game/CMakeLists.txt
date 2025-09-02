cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Minecraft_Clone ${GAME_SOURCES})

target_include_directories(Minecraft_Clone
    PRIVATE
        ${CMAKE_SOURCE_DIR}/app/game/include
        ${CMAKE_SOURCE_DIR}/include          # <-- root include for "core/Camera.hpp"
)


# Link engine first
target_link_libraries(Minecraft_Clone
    PRIVATE
        engine
        glfw
        glad
        opengl32    # Windows OpenGL library
)
