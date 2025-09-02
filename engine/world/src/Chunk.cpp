// Chunk.cpp
// Implementation of Chunk. Generates Perlin-based heights, builds a triangle
// mesh of only visible faces, and builds a separate line mesh for outlines.
//
// The outline mesh contains pairs of vertices (line segments). The line color
// is black (0,0,0) and thickness is configurable via SetOutlineThickness().

#include "Chunk.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>

// PerlinNoise implementation header (from external/perlin)
#include "PerlinNoise.hpp"

// Layer colors indexed by y / 3 (simple banding)
static const glm::vec3 layerColors[] = {
    {0.0f, 0.2f, 0.7f},   // water
    {0.9f, 0.85f, 0.6f},  // sand
    {0.2f, 0.7f, 0.2f},   // grass
    {0.45f,0.33f,0.21f},  // dirt
    {0.5f,0.5f,0.5f},     // stone
    {0.85f,0.85f,0.85f},  // rock
    {1.0f,1.0f,1.0f}      // snow
};

// Adjustable outline thickness (pixels). Change using SetOutlineThickness().
// Default: 1.5f
static float g_outlineThickness = 0.00001f;
void Chunk::SetOutlineThickness(float t) { g_outlineThickness = t; }

// -----------------------------
// Construction / Destruction
// -----------------------------
Chunk::Chunk(int originX_, int originZ_, int sizeX_, int sizeZ_)
    : originX(originX_), originZ(originZ_), sizeX(sizeX_), sizeZ(sizeZ_), maxHeight(64) {
    heights.assign(static_cast<size_t>(sizeX) * static_cast<size_t>(sizeZ), 1);
    GenerateHeightmapWithPerlin();
}

Chunk::~Chunk() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (outlineVBO) glDeleteBuffers(1, &outlineVBO);
    if (outlineVAO) glDeleteVertexArrays(1, &outlineVAO);
}

// -----------------------------
// Heightmap generation (Perlin)
// -----------------------------
void Chunk::GenerateHeightmapWithPerlin() {
    // deterministic seed (change for different worlds)
    siv::PerlinNoise perlin(123456);
    const double freq = 0.05;

    for (int x = 0; x < sizeX; ++x) {
        for (int z = 0; z < sizeZ; ++z) {
            double nx = double(originX + x) * freq;
            double nz = double(originZ + z) * freq;
            double n = perlin.noise2D_01(nx, nz); // returns [0,1]
            n = std::clamp(n, 0.0, 1.0);
            int h = static_cast<int>(n * 16.0) + 1; // heights roughly 1..17
            h = std::min(h, maxHeight);
            heights[static_cast<size_t>(x) + static_cast<size_t>(z) * static_cast<size_t>(sizeX)] = h;
        }
    }
}

// -----------------------------
// Helper: check solid block at world coords
// -----------------------------
bool Chunk::IsSolidAt(int worldX, int worldY, int worldZ) const {
    int lx = worldX - originX;
    int lz = worldZ - originZ;
    if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) return false;
    int h = heights[static_cast<size_t>(lx) + static_cast<size_t>(lz) * static_cast<size_t>(sizeX)];
    return (worldY >= 0 && worldY < h);
}

// -----------------------------
// Vertex append helper (position(3) color(3))
// -----------------------------
static inline void appendVertex(std::vector<float>& dst, float px, float py, float pz, const glm::vec3& col) {
    dst.push_back(px);
    dst.push_back(py);
    dst.push_back(pz);
    dst.push_back(col.r);
    dst.push_back(col.g);
    dst.push_back(col.b);
}

// face triangle vertex offsets (6 verts per face; two triangles)
static const float facePositions[6][18] = {
    // +X
    {0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f},
     // -X
     {-0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
      -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f},
      // +Y (top)
      {-0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f},
        // -Y (bottom)
        {-0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f},
          // +Z
          {-0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f},
            // -Z
            {0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
             -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f}
};

// corners per face (4 unique corners) used to emit line segments
static const float faceCorners[6][12] = {
    { 0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f },
    {-0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f},
    {-0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f },
    {-0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f},
    {-0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f },
    { 0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f }
};

// -----------------------------
// Build mesh: only emit faces that are visible (neighbor missing).
// Also build outline segments for each emitted face.
// -----------------------------
void Chunk::BuildMesh() {
    meshData.clear();
    outlineMeshData.clear();

    const glm::vec3 kBlack(0.0f, 0.0f, 0.0f);

    for (int x = 0; x < sizeX; ++x) {
        for (int z = 0; z < sizeZ; ++z) {
            int h = heights[static_cast<size_t>(x) + static_cast<size_t>(z) * static_cast<size_t>(sizeX)];
            for (int y = 0; y < h; ++y) {
                // neighbor presence (within chunk). Out-of-range considered empty -> face visible.
                bool neighborPosX = (x + 1 < sizeX && y < heights[static_cast<size_t>(x + 1) + static_cast<size_t>(z) * static_cast<size_t>(sizeX)]);
                bool neighborNegX = (x - 1 >= 0 && y < heights[static_cast<size_t>(x - 1) + static_cast<size_t>(z) * static_cast<size_t>(sizeX)]);
                bool neighborPosZ = (z + 1 < sizeZ && y < heights[static_cast<size_t>(x) + static_cast<size_t>(z + 1) * static_cast<size_t>(sizeX)]);
                bool neighborNegZ = (z - 1 >= 0 && y < heights[static_cast<size_t>(x) + static_cast<size_t>(z - 1) * static_cast<size_t>(sizeX)]);
                bool neighborPosY = (y + 1 < h);
                bool neighborNegY = (y - 1 >= 0);

                int colorIndex = std::min(static_cast<int>(sizeof(layerColors) / sizeof(layerColors[0])) - 1, y / 3);
                glm::vec3 color = layerColors[colorIndex];

                // lambda to append a single face's triangles and its outline edges
                auto emitFace = [&](int faceIdx) {
                    // triangles (6 vertices -> two tris)
                    for (int v = 0; v < 6; ++v) {
                        int base = v * 3;
                        appendVertex(meshData,
                            originX + x + facePositions[faceIdx][base + 0],
                            static_cast<float>(y) + facePositions[faceIdx][base + 1],
                            originZ + z + facePositions[faceIdx][base + 2],
                            color);
                    }
                    // outline: 4 edges -> 4 line segments -> 8 vertices (pairs)
                    for (int e = 0; e < 4; ++e) {
                        int i0 = e;
                        int i1 = (e + 1) % 4;
                        int b0 = i0 * 3;
                        int b1 = i1 * 3;
                        appendVertex(outlineMeshData,
                            originX + x + faceCorners[faceIdx][b0 + 0],
                            static_cast<float>(y) + faceCorners[faceIdx][b0 + 1],
                            originZ + z + faceCorners[faceIdx][b0 + 2],
                            kBlack);
                        appendVertex(outlineMeshData,
                            originX + x + faceCorners[faceIdx][b1 + 0],
                            static_cast<float>(y) + faceCorners[faceIdx][b1 + 1],
                            originZ + z + faceCorners[faceIdx][b1 + 2],
                            kBlack);
                    }
                    };

                if (!neighborPosX) emitFace(0);
                if (!neighborNegX) emitFace(1);
                if (!neighborPosY) emitFace(2);
                if (!neighborNegY) emitFace(3);
                if (!neighborPosZ) emitFace(4);
                if (!neighborNegZ) emitFace(5);
            }
        }
    }

    uploadMesh();
}

// -----------------------------
// Upload VBO/VAO for triangles and for outlines
// -----------------------------
void Chunk::uploadMesh() {
    // --- Filled triangles ---
    if (VAO == 0) glGenVertexArrays(1, &VAO);
    if (VBO == 0) glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (!meshData.empty())
        glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    // position(3) then color(3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // --- Outline lines ---
    if (outlineVAO == 0) glGenVertexArrays(1, &outlineVAO);
    if (outlineVBO == 0) glGenBuffers(1, &outlineVBO);

    glBindVertexArray(outlineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    if (!outlineMeshData.empty())
        glBufferData(GL_ARRAY_BUFFER, outlineMeshData.size() * sizeof(float), outlineMeshData.data(), GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    // position(3) color(3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

// -----------------------------
// Draw both meshes: filled triangles first (polygon offset), then outlines (lines).
// Outline thickness uses the static g_outlineThickness.
// -----------------------------
void Chunk::Draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    if (meshData.empty()) return;

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;
    unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // Draw filled geometry with polygon offset so lines sit cleanly on top
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    glBindVertexArray(VAO);
    GLsizei triVertexCount = static_cast<GLsizei>(meshData.size() / 6);
    glDrawArrays(GL_TRIANGLES, 0, triVertexCount);
    glBindVertexArray(0);

    glDisable(GL_POLYGON_OFFSET_FILL);

    // Draw outlines (lines)
    if (!outlineMeshData.empty()) {
        glBindVertexArray(outlineVAO);
        // Set line width; some drivers clamp to 1.0. Pick a value you like via SetOutlineThickness()
        glLineWidth(g_outlineThickness);
        GLsizei lineVertexCount = static_cast<GLsizei>(outlineMeshData.size() / 6);
        glDrawArrays(GL_LINES, 0, lineVertexCount);
        glBindVertexArray(0);
    }
}

// -----------------------------
// Returns every solid block's min corner position (world coords).
// Useful for debug or tools. Not used for collision here.
// -----------------------------
std::vector<glm::vec3> Chunk::GetSolidBlockPositions() const {
    std::vector<glm::vec3> positions;
    positions.reserve(static_cast<size_t>(sizeX) * static_cast<size_t>(sizeZ) * 4);

    for (int x = 0; x < sizeX; ++x) {
        for (int z = 0; z < sizeZ; ++z) {
            int h = heights[static_cast<size_t>(x) + static_cast<size_t>(z) * static_cast<size_t>(sizeX)];
            for (int y = 0; y < h; ++y) {
                positions.emplace_back(static_cast<float>(originX + x),
                    static_cast<float>(y),
                    static_cast<float>(originZ + z));
            }
        }
    }
    return positions;
}
