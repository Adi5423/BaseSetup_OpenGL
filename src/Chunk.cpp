#include "Chunk.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Perlin header (must exist at external/perlin/PerlinNoise.hpp)
#include "PerlinNoise.hpp"

// Simple mapping from y to colors
static const glm::vec3 layerColors[] = {
    {0.0f, 0.2f, 0.7f}, // water
    {0.9f, 0.85f, 0.6f}, // sand
    {0.2f, 0.7f, 0.2f}, // grass
    {0.45f,0.33f,0.21f}, // dirt
    {0.5f,0.5f,0.5f}, // stone
    {0.85f,0.85f,0.85f}, // rock
    {1.0f,1.0f,1.0f} // snow
};

Chunk::Chunk(int originX, int originZ, int sizeX, int sizeZ)
    : originX(originX), originZ(originZ), sizeX(sizeX), sizeZ(sizeZ), maxHeight(64) {
    heights.resize(sizeX * sizeZ, 1);
    GenerateHeightmapWithPerlin();
}

Chunk::~Chunk() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

void Chunk::GenerateHeightmapWithPerlin() {
    // seed for determinism; you can randomize
    siv::PerlinNoise perlin(123456);
    double freq = 0.05; // tweak for larger/smaller hills

    for (int x = 0; x < sizeX; ++x) {
        for (int z = 0; z < sizeZ; ++z) {
            double nx = double(originX + x) * freq;
            double nz = double(originZ + z) * freq;
            double n = perlin.noise2D_01(nx, nz);  // [0, 1] range
            // normalize n [-1,1] -> [0,1] if needed (KdotJPG's PerlinNoise returns [-1,1] or [0,1]? adjust)
            // assume [0,1] for safety; clamp and scale
            if (n < 0.0) n = 0.0; if (n > 1.0) n = 1.0;
            int h = static_cast<int>(n * 16.0) + 1; // heights 1..17
            if (h > maxHeight) h = maxHeight;
            heights[x + z * sizeX] = h;
        }
    }
}

static inline void appendVertex(std::vector<float>& dst, float px, float py, float pz, const glm::vec3& col) {
    dst.push_back(px); dst.push_back(py); dst.push_back(pz);
    dst.push_back(col.r); dst.push_back(col.g); dst.push_back(col.b);
}

// Each face is 6 vertices (two tris). Provide face relative positions.
static const float facePositions[6][18] = {
    // +X face
    {0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 0.5f,
     0.5f,  0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, -0.5f, -0.5f},
     // -X
     {-0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f},
      // +Y
      {-0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f,  0.5f,
        0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f, -0.5f},
        // -Y
        {-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f},
          // +Z
          {-0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f,  0.5f, 0.5f,
           0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f, -0.5f, 0.5f},
           // -Z
           {0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f}
};

void Chunk::BuildMesh() {
    meshData.clear();

    // For each cell (x,z) with height h, and y in [0,h-1], add only faces with missing neighbors
    for (int x = 0; x < sizeX; ++x) {
        for (int z = 0; z < sizeZ; ++z) {
            int h = heights[x + z * sizeX];
            for (int y = 0; y < h; ++y) {
                // neighbors presence checks (within chunk or consider out-of-range as empty)
                bool neighborPosX = (x + 1 < sizeX && y < heights[(x + 1) + z * sizeX]);
                bool neighborNegX = (x - 1 >= 0 && y < heights[(x - 1) + z * sizeX]);
                bool neighborPosZ = (z + 1 < sizeZ && y < heights[x + (z + 1) * sizeX]);
                bool neighborNegZ = (z - 1 >= 0 && y < heights[x + (z - 1) * sizeX]);
                bool neighborPosY = (y + 1 < h);
                bool neighborNegY = (y - 1 >= 0);

                glm::vec3 color = layerColors[glm::clamp(y / 3, 0, (int)(sizeof(layerColors) / sizeof(layerColors[0]) - 1))];

                // for each of 6 faces, if neighbor in that direction is missing -> append face
                if (!neighborPosX) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[0][base + 0],
                            y + facePositions[0][base + 1],
                            originZ + z + facePositions[0][base + 2],
                            color);
                    }
                }
                if (!neighborNegX) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[1][base + 0],
                            y + facePositions[1][base + 1],
                            originZ + z + facePositions[1][base + 2],
                            color);
                    }
                }
                if (!neighborPosY) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[2][base + 0],
                            y + facePositions[2][base + 1],
                            originZ + z + facePositions[2][base + 2],
                            color);
                    }
                }
                if (!neighborNegY) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[3][base + 0],
                            y + facePositions[3][base + 1],
                            originZ + z + facePositions[3][base + 2],
                            color);
                    }
                }
                if (!neighborPosZ) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[4][base + 0],
                            y + facePositions[4][base + 1],
                            originZ + z + facePositions[4][base + 2],
                            color);
                    }
                }
                if (!neighborNegZ) {
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[5][base + 0],
                            y + facePositions[5][base + 1],
                            originZ + z + facePositions[5][base + 2],
                            color);
                    }
                }
            }
        }
    }

    uploadMesh();
}

void Chunk::uploadMesh() {
    if (VAO == 0) glGenVertexArrays(1, &VAO);
    if (VBO == 0) glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    // position(3 floats) then color(3 floats) => stride = 6 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Chunk::Draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    if (meshData.empty()) return;

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    // model for chunk origin is identity, we already baked world coords into mesh vertices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;
    unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(meshData.size() / 6)); // vertex count = floats/6
    glBindVertexArray(0);
}
