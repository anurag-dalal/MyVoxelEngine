#ifndef SPARSE_VOXEL_OCTREE_H
#define SPARSE_VOXEL_OCTREE_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <array>

struct OctreeNode {
    glm::vec3 min;
    glm::vec3 max;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    std::vector<std::pair<glm::vec3, unsigned int>> voxels;
    bool isLeaf;

    OctreeNode(const glm::vec3& minPos, const glm::vec3& maxPos) 
        : min(minPos), max(maxPos), isLeaf(true) {}
};

class SparseVoxelOctree {
public:
    SparseVoxelOctree(const glm::vec3& worldMin, const glm::vec3& worldMax, float voxelSize);
    ~SparseVoxelOctree() = default;

    void insertVoxel(const glm::vec3& position, unsigned int blockId);
    std::vector<std::pair<glm::vec3, unsigned int>> getVisibleVoxels(const glm::mat4& viewProjection) const;

private:
    std::unique_ptr<OctreeNode> root;
    float minNodeSize;
    
    void subdivideNode(OctreeNode* node);
    bool isVisible(const OctreeNode* node, const glm::mat4& viewProjection) const;
    void collectVisibleVoxels(const OctreeNode* node, const glm::mat4& viewProjection, 
                             std::vector<std::pair<glm::vec3, unsigned int>>& result) const;
    int getOctant(const glm::vec3& position, const OctreeNode* node) const;
};

#endif // SPARSE_VOXEL_OCTREE_H