#include "Physics.hpp"
#include <algorithm>

bool AABBOverlap(const AABB& a, const AABB& b) {
    if (a.max.x <= b.min.x || a.min.x >= b.max.x) return false;
    if (a.max.y <= b.min.y || a.min.y >= b.max.y) return false;
    if (a.max.z <= b.min.z || a.min.z >= b.max.z) return false;
    return true;
}

glm::vec3 ComputeMTV(const AABB& a, const AABB& b) {
    // compute overlap along each axis (positive values)
    float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
    float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);
    float overlapZ = std::min(a.max.z, b.max.z) - std::max(a.min.z, b.min.z);

    // pick smallest overlap axis to resolve
    if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) return glm::vec3(0.0f);

    if (overlapX <= overlapY && overlapX <= overlapZ) {
        // move in x
        float dir = (a.min.x + a.max.x) * 0.5f < (b.min.x + b.max.x) * 0.5f ? -1.0f : 1.0f;
        return glm::vec3(overlapX * dir, 0.0f, 0.0f);
    }
    else if (overlapY <= overlapX && overlapY <= overlapZ) {
        float dir = (a.min.y + a.max.y) * 0.5f < (b.min.y + b.max.y) * 0.5f ? -1.0f : 1.0f;
        return glm::vec3(0.0f, overlapY * dir, 0.0f);
    }
    else {
        float dir = (a.min.z + a.max.z) * 0.5f < (b.min.z + b.max.z) * 0.5f ? -1.0f : 1.0f;
        return glm::vec3(0.0f, 0.0f, overlapZ * dir);
    }
}

void ResolvePlayerCollisions(glm::vec3& pos, glm::vec3& vel, bool& grounded,
    float radius, float height,
    const std::vector<glm::ivec3>& blocks) {
    grounded = false;
    // start with player AABB at proposed pos
    AABB player = PlayerAABB(pos, radius, height);

    // we will iterate and resolve repeatedly because resolution can create new overlaps on other axes
    const int maxIterations = 4;
    for (int iter = 0; iter < maxIterations; ++iter) {
        bool any = false;
        player = PlayerAABB(pos, radius, height);

        for (const auto& b : blocks) {
            AABB blockA = BlockAABB(b.x, b.y, b.z);
            if (!AABBOverlap(player, blockA)) continue;

            glm::vec3 mtv = ComputeMTV(player, blockA);
            if (mtv == glm::vec3(0.0f)) continue;

            // apply separation: move pos by mtv
            pos += mtv;
            any = true;

            // zero velocity component along the axis we resolved
            if (std::abs(mtv.x) > 1e-6f) vel.x = 0.0f;
            if (std::abs(mtv.y) > 1e-6f) {
                if (mtv.y > 0) {
                    // moved up -> we were below block (rare), don't set grounded
                }
                else {
                    // moved down -> player placed on top of block
                    grounded = true;
                    vel.y = 0.0f;
                }
            }
            if (std::abs(mtv.z) > 1e-6f) vel.z = 0.0f;

            // update player AABB after move before checking next block
            player = PlayerAABB(pos, radius, height);
        }

        if (!any) break;
    }
}
