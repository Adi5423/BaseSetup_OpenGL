#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>

class Chunk {
public:
    Chunk(int originX, int originZ, int sizeX = 32, int sizeZ = 32);
    ~Chunk();

    void GenerateHeightmapWithPerlin(); // fills heights[]
    void BuildMesh();                   // builds meshData & uploads VBO/VAO
    void Draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection);

private:
    int originX, originZ;
    int sizeX, sizeZ;
    int maxHeight;

    std::vector<int> heights; // sizeX * sizeZ
    std::vector<float> meshData; // interleaved: position(3) color(3)

    unsigned int VAO = 0, VBO = 0;
    void uploadMesh();
};
