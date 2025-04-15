#pragma once

#include "../World/Voxel.h"
#include <vector>
#include <glm/glm.hpp>

class Model {
protected:
    std::vector<Voxel> voxels;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

public:
    Model(const glm::vec3& pos = glm::vec3(0.0f), 
          const glm::vec3& rot = glm::vec3(0.0f),
          const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {}
    
    virtual ~Model() = default;

    // Get all voxels in world space
    virtual std::vector<Voxel> getVoxels() const {
        std::vector<Voxel> worldVoxels;
        for (const auto& voxel : voxels) {
            // Transform voxel position by model's transform
            glm::vec3 worldPos = position + voxel.position;
            worldVoxels.emplace_back(worldPos, voxel.blockId);
        }
        return worldVoxels;
    }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(const glm::vec3& rot) { rotation = rot; }
    void setScale(const glm::vec3& scl) { scale = scl; }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }
};