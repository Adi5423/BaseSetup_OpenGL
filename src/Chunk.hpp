// Chunk.hpp
// Represents a single chunk of voxels (columns of integer heights).
// Provides a triangle mesh for visible faces and a separate "outline" line mesh
// that draws black borders around faces for visual separation.
//
// To change border thickness globally: call Chunk::SetOutlineThickness(yourValue)
// before rendering (or set it once in main after start).

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>

class Chunk {
public:
    // ctor: originX/Z are world coordinates of the chunk's (0,0) corner.
    Chunk(int originX, int originZ, int sizeX = 32, int sizeZ = 32);
    ~Chunk();

    // Generates a heightmap using Perlin noise (fills heights vector).
    void GenerateHeightmapWithPerlin();

    // BuildMesh populates meshData and outlineMeshData and uploads buffers.
    void BuildMesh();

    // Draws both filled triangles and outlines (calls glDrawArrays).
    void Draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection);

    // Query: is there a solid block at world (x,y,z)?
    bool IsSolidAt(int worldX, int worldY, int worldZ) const;

    // Returns world (x,y,z) of every solid block's min-corner (useful for debug).
    std::vector<glm::vec3> GetSolidBlockPositions() const;

    // Outline thickness control (pixels). Default value defined in Chunk.cpp.
    static void SetOutlineThickness(float t);

private:
    int originX, originZ;
    int sizeX, sizeZ;
    int maxHeight;

    std::vector<int> heights;            // sizeX * sizeZ
    std::vector<float> meshData;         // interleaved: pos(3) color(3) for filled triangles
    std::vector<float> outlineMeshData;  // interleaved: pos(3) color(3) for line segments

    unsigned int VAO = 0, VBO = 0;
    unsigned int outlineVAO = 0, outlineVBO = 0;

    void uploadMesh(); // uploads both VBOs/VAOs
};
