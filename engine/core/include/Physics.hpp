#pragma once
#include <glm/glm.hpp>
#include <vector>

// Axis-aligned bounding box
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

// Create block AABB at integer coordinates (x,y,z) with size 1
inline AABB BlockAABB(int bx, int by, int bz) {
    return AABB{ glm::vec3((float)bx, (float)by, (float)bz), glm::vec3((float)bx + 1.0f, (float)by + 1.0f, (float)bz + 1.0f) };
}

// Create player AABB from center position, radius and height
inline AABB PlayerAABB(const glm::vec3& pos, float radius, float height) {
    float halfWidth = radius;
    return AABB{ glm::vec3(pos.x - halfWidth, pos.y, pos.z - halfWidth), glm::vec3(pos.x + halfWidth, pos.y + height, pos.z + halfWidth) };
}

// returns true if overlap
bool AABBOverlap(const AABB& a, const AABB& b);

// compute minimal translation vector to separate a from b (a moved out of b). 
// Returns vector that when added to `a` (move a by this) will just separate them.
glm::vec3 ComputeMTV(const AABB& a, const AABB& b);

// Resolve player collisions against a list of block coordinates (vector of ints triples).
// blocks: list of (bx,by,bz) integer block coordinates considered solid
// pos/vel/grounded are in-out parameters. radius/height are player parameters.
// The function will modify pos and vel to avoid penetrating blocks and set grounded true if standing on a block.
void ResolvePlayerCollisions(glm::vec3& pos, glm::vec3& vel, bool& grounded,
    float radius, float height,
    const std::vector<glm::ivec3>& blocks);
