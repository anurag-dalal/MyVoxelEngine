#include "SparseVoxelOctree.h"
#include <glm/gtc/matrix_access.hpp>

SparseVoxelOctree::SparseVoxelOctree(const glm::vec3& worldMin, const glm::vec3& worldMax, float voxelSize)
    : minNodeSize(voxelSize * 2.0f) {
    root = std::make_unique<OctreeNode>(worldMin, worldMax);
}

void SparseVoxelOctree::insertVoxel(const glm::vec3& position, unsigned int blockId) {
    OctreeNode* current = root.get();
    
    while ((current->max.x - current->min.x) > minNodeSize) {
        if (current->isLeaf) {
            if (current->voxels.size() < 8) {
                current->voxels.emplace_back(position, blockId);
                return;
            }
            subdivideNode(current);
        }
        
        int octant = getOctant(position, current);
        if (!current->children[octant]) {
            glm::vec3 mid = (current->min + current->max) * 0.5f;
            glm::vec3 childMin = current->min;
            glm::vec3 childMax = current->max;
            
            if (octant & 1) childMin.x = mid.x; else childMax.x = mid.x;
            if (octant & 2) childMin.y = mid.y; else childMax.y = mid.y;
            if (octant & 4) childMin.z = mid.z; else childMax.z = mid.z;
            
            current->children[octant] = std::make_unique<OctreeNode>(childMin, childMax);
        }
        current = current->children[octant].get();
    }
    
    current->voxels.emplace_back(position, blockId);
}

void SparseVoxelOctree::subdivideNode(OctreeNode* node) {
    glm::vec3 mid = (node->min + node->max) * 0.5f;
    
    // Move existing voxels to children
    for (const auto& voxel : node->voxels) {
        int octant = getOctant(voxel.first, node);
        if (!node->children[octant]) {
            glm::vec3 childMin = node->min;
            glm::vec3 childMax = node->max;
            
            if (octant & 1) childMin.x = mid.x; else childMax.x = mid.x;
            if (octant & 2) childMin.y = mid.y; else childMax.y = mid.y;
            if (octant & 4) childMin.z = mid.z; else childMax.z = mid.z;
            
            node->children[octant] = std::make_unique<OctreeNode>(childMin, childMax);
        }
        node->children[octant]->voxels.push_back(voxel);
    }
    
    node->voxels.clear();
    node->isLeaf = false;
}

int SparseVoxelOctree::getOctant(const glm::vec3& position, const OctreeNode* node) const {
    glm::vec3 mid = (node->min + node->max) * 0.5f;
    int octant = 0;
    if (position.x >= mid.x) octant |= 1;
    if (position.y >= mid.y) octant |= 2;
    if (position.z >= mid.z) octant |= 4;
    return octant;
}

bool SparseVoxelOctree::isVisible(const OctreeNode* node, const glm::mat4& viewProjection) const {
    std::array<glm::vec4, 8> corners = {
        glm::vec4(node->min.x, node->min.y, node->min.z, 1.0f),
        glm::vec4(node->max.x, node->min.y, node->min.z, 1.0f),
        glm::vec4(node->min.x, node->max.y, node->min.z, 1.0f),
        glm::vec4(node->max.x, node->max.y, node->min.z, 1.0f),
        glm::vec4(node->min.x, node->min.y, node->max.z, 1.0f),
        glm::vec4(node->max.x, node->min.y, node->max.z, 1.0f),
        glm::vec4(node->min.x, node->max.y, node->max.z, 1.0f),
        glm::vec4(node->max.x, node->max.y, node->max.z, 1.0f)
    };

    // Check if any corner is inside the view frustum
    for (const auto& corner : corners) {
        glm::vec4 clipSpace = viewProjection * corner;
        if (clipSpace.w <= 0.0f) continue; // Behind camera
        
        clipSpace /= clipSpace.w;
        if (clipSpace.x >= -1.0f && clipSpace.x <= 1.0f &&
            clipSpace.y >= -1.0f && clipSpace.y <= 1.0f &&
            clipSpace.z >= -1.0f && clipSpace.z <= 1.0f) {
            return true;
        }
    }

    // Check if frustum intersects the box (simplified test)
    glm::vec3 center = (node->min + node->max) * 0.5f;
    glm::vec3 extents = (node->max - node->min) * 0.5f;
    glm::vec4 centerClip = viewProjection * glm::vec4(center, 1.0f);
    
    if (centerClip.w <= 0.0f) return false;
    centerClip /= centerClip.w;
    
    float maxRadius = glm::length(extents);
    return glm::abs(centerClip.x) <= 1.0f + maxRadius &&
           glm::abs(centerClip.y) <= 1.0f + maxRadius &&
           glm::abs(centerClip.z) <= 1.0f + maxRadius;
}

std::vector<std::pair<glm::vec3, unsigned int>> SparseVoxelOctree::getVisibleVoxels(const glm::mat4& viewProjection) const {
    std::vector<std::pair<glm::vec3, unsigned int>> visibleVoxels;
    visibleVoxels.reserve(1024); // Pre-allocate space for better performance
    collectVisibleVoxels(root.get(), viewProjection, visibleVoxels);
    return visibleVoxels;
}

void SparseVoxelOctree::collectVisibleVoxels(const OctreeNode* node, const glm::mat4& viewProjection,
                                            std::vector<std::pair<glm::vec3, unsigned int>>& result) const {
    if (!node || !isVisible(node, viewProjection)) {
        return;
    }

    if (node->isLeaf) {
        result.insert(result.end(), node->voxels.begin(), node->voxels.end());
    } else {
        for (const auto& child : node->children) {
            if (child) {
                collectVisibleVoxels(child.get(), viewProjection, result);
            }
        }
    }
}