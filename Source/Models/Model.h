#pragma once

#include "../World/Voxel.h"
#include <vector>
#include <glm/glm.hpp>

class Model {
protected:
    std::vector<Voxel> voxels;
    glm::vec3 position;
    glm::vec3 rotation;
    float scale;

public:
    Model(const glm::vec3& pos = glm::vec3(0.0f), 
          const glm::vec3& rot = glm::vec3(0.0f),
          const float scl = 0.1f)
        : position(pos), rotation(rot), scale(scl) {}
    
    virtual ~Model() = default;

    // Get all voxels in world space
    virtual std::vector<Voxel> getVoxels() const {
        std::vector<Voxel> worldVoxels;
        for (const auto& voxel : voxels) {
            // Scale first, then translate
            glm::vec3 worldPos = position + (voxel.position * scale);
            worldVoxels.emplace_back(worldPos, voxel.blockId);
        }
        return worldVoxels;
    }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(const glm::vec3& rot) { rotation = rot; }
    void setScale(const float scl) { scale = scl; }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    float getScale() const { return scale; }
};