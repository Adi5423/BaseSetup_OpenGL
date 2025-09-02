#include "Collision.hpp"

bool CheckCollision(const glm::vec3& nextPos, const std::vector<glm::vec3>& blocks, float playerRadius) {
    glm::vec3 playerMin = nextPos - glm::vec3(playerRadius, 0.0f, playerRadius);
    glm::vec3 playerMax = nextPos + glm::vec3(playerRadius, 1.8f, playerRadius); // approx player height

    for (const auto& block : blocks) {
        glm::vec3 blockMin = block;
        glm::vec3 blockMax = block + glm::vec3(1.0f);

        bool overlapX = (playerMin.x < blockMax.x) && (playerMax.x > blockMin.x);
        bool overlapY = (playerMin.y < blockMax.y) && (playerMax.y > blockMin.y);
        bool overlapZ = (playerMin.z < blockMax.z) && (playerMax.z > blockMin.z);

        if (overlapX && overlapY && overlapZ)
            return true;
    }
    return false;
}
