#pragma once
#include <glm/glm.hpp>
#include <vector>

// Checks if player collides with any block AABB.
// playerRadius defaults to 0.3f, player height within function is 1.8f.
bool CheckCollision(const glm::vec3& nextPos, const std::vector<glm::vec3>& blocks, float playerRadius = 0.3f);
