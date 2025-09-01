// main.cpp
// Entry point: creates window, compiles shader, creates a Chunk and renders it.
// Movement: WASD + mouse look. Hold Left Shift to sprint.
// No collisions here (you can go below/through terrain).

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Camera.hpp"
#include "Chunk.hpp"

// -----------------------------
// Globals (camera, timing, window)
// -----------------------------
static Camera camera(glm::vec3(16.0f, 20.0f, 40.0f)); // spawn above chunk
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static float lastX = 1280.0f / 2.0f;
static float lastY = 720.0f / 2.0f;
static bool firstMouse = true;

const int WIN_WIDTH = 1280;
const int WIN_HEIGHT = 720;

// -----------------------------
// Mouse callback - forwards offsets to camera
// -----------------------------
static void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // inverted Y for FPS feel
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// -----------------------------
// Input processing: WASD + SHIFT sprint
// IMPORTANT: No clamping on Y here -> you can move below ground freely.
// -----------------------------
static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Base movement speed (camera.MovementSpeed is configurable in Camera)
    float sprintFactor = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 2.0f : 1.0f;
    float speed = camera.MovementSpeed * sprintFactor;

    glm::vec3 nextPos = camera.Position;

    // Move in horizontal plane only (preserve camera.Position.y for vertical freedom)
    glm::vec3 flatFront = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 flatRight = glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z));


    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.Position.y -= camera.MovementSpeed * deltaTime; // move up
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.Position.y += camera.MovementSpeed * deltaTime; // move down
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        nextPos += flatFront * (speed * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        nextPos -= flatFront * (speed * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        nextPos -= flatRight * (speed * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        nextPos += flatRight * (speed * deltaTime);

    // preserve current Y (no auto-clamp)
    nextPos.y = camera.Position.y;

    // Apply movement immediately (no collision).
    camera.Position = nextPos;
}

// -----------------------------
// Minimal shader sources (position + color)
// -----------------------------
static const char* vertexShaderSource = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform mat4 u_MVP;
void main() {
    vColor = aColor;
    gl_Position = u_MVP * vec4(aPos, 1.0);
}
)glsl";

static const char* fragmentShaderSource = R"glsl(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)glsl";

// -----------------------------
// Helper: compile shader
// -----------------------------
static unsigned int CompileShader(GLenum type, const char* src) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "Shader compile error: " << log << "\n";
    }
    return sh;
}

// -----------------------------
// main()
// -----------------------------
int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed\n";
        return -1;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Minecraft_Clone - Outlined Voxels", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync on

    // Input callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    // Compile & link shaders
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // Check link status
    int ok = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, log);
        std::cerr << "Program link error: " << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glEnable(GL_DEPTH_TEST);

    // Adjust default outline thickness here if you want a different starting value:
    // Chunk::SetOutlineThickness(2.0f);
    // Default is defined inside Chunk.cpp (1.5f by default).

    // Create a single chunk at origin
    const int CHUNK_SIZE = 32;
    Chunk chunk(0, 0, CHUNK_SIZE, CHUNK_SIZE);
    chunk.BuildMesh(); // generates mesh and outlines

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window);

        // Render
        glClearColor(0.53f, 0.80f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute MVP
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 500.0f);

        // Draw chunk
        chunk.Draw(shaderProgram, view, projection);

        glfwSwapBuffers(window);
    }

    // Cleanup and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
