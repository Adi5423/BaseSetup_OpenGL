#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "Camera.hpp"
#include "Chunk.hpp"

// --- Globals ---
static Camera camera(glm::vec3(16.0f, 20.0f, 40.0f)); // spawn above chunk
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static float lastX = 1280.0f / 2.0f;
static float lastY = 720.0f / 2.0f;
static bool firstMouse = true;

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = static_cast<float>(ypos) - lastY;
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    camera.ProcessMouseMovement(xoffset, yoffset);
}

static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

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

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    // GL 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Minecraft_Clone - Perlin Terrain", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    // Basic shader compile/link
    auto compile = [](GLenum type, const char* src) -> unsigned int {
        unsigned int sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        int ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024]; glGetShaderInfoLog(sh, 1024, nullptr, log);
            std::cerr << "Shader compile error: " << log << "\n";
        }
        return sh;
        };

    unsigned int vs = compile(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    int ok; glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(shaderProgram, 1024, nullptr, log); std::cerr << "Link error: " << log << "\n"; }
    glDeleteShader(vs); glDeleteShader(fs);

    // Depth test
    glEnable(GL_DEPTH_TEST);

    // Create one chunk at origin (originX, originZ), size 32x32
    const int CHUNK_SIZE = 32;
    Chunk chunk(0, 0, CHUNK_SIZE, CHUNK_SIZE);
    chunk.BuildMesh();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window);

        // Render
        glClearColor(0.53f, 0.80f, 0.92f, 1.0f); // sky-like
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), 1280.0f / 720.0f, 0.1f, 500.0f);

        chunk.Draw(shaderProgram, view, projection);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
