#pragma once

#include "Model.h"
#include <random>

class Tree : public Model {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<> trunkHeightDist;
    static std::uniform_int_distribution<> crownSizeDist;

public:
    Tree(const glm::vec3& pos = glm::vec3(0.0f), float voxelScale = 0.1f) 
        : Model(pos, glm::vec3(0.0f), voxelScale) {
        generateTree();
    }

private:
    void generateTree() {
        // Generate random tree dimensions
        int trunkHeight = trunkHeightDist(gen);
        int crownSize = crownSizeDist(gen);

        // Create trunk (wood blocks)
        for(int y = 0; y < trunkHeight; ++y) {
            voxels.emplace_back(glm::vec3(0.0f, y * 1.0f, 0.0f), 4); // Wood block ID = 4
        }

        // Create leaves
        for(int y = trunkHeight - 2; y < trunkHeight + crownSize; ++y) {
            for(int x = -crownSize/2; x <= crownSize/2; ++x) {
                for(int z = -crownSize/2; z <= crownSize/2; ++z) {
                    // Skip if too far from trunk (make crown roughly spherical)
                    float distance = sqrt(x*x + (y-trunkHeight)*(y-trunkHeight) + z*z);
                    if(distance <= crownSize/2.0f + 1.0f) {
                        voxels.emplace_back(glm::vec3(x * 1.0f, y * 1.0f, z * 1.0f), 5); // Leaves block ID = 5
                    }
                }
            }
        }
    }
};