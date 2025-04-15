#ifndef BLOCK_DATABASE_H
#define BLOCK_DATABASE_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

enum class BlockFace {
    TOP,
    BOTTOM,
    FRONT,
    BACK,
    LEFT,
    RIGHT
};

enum class BlockType : unsigned int {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    WOOD_LOG = 4,
    LEAVES = 5,
    SAND = 6,
    WATER = 7
};

struct BlockTexture {
    glm::vec2 textureCoords;  // UV coordinates in the texture atlas
    bool isTransparent;       // Whether this texture has transparency

    BlockTexture(const glm::vec2& coords = glm::vec2(0, 0), bool transparent = false)
        : textureCoords(coords), isTransparent(transparent) {}
};

class BlockDatabase {
public:
    static BlockDatabase& getInstance();
    
    // Get texture coordinates for a specific block face
    BlockTexture getBlockTexture(BlockType type, BlockFace face) const;
    
    // Check if a block type is transparent
    bool isBlockTransparent(BlockType type) const;
    
    // Get the name of a block type (for debugging/UI)
    std::string getBlockName(BlockType type) const;

private:
    BlockDatabase();  // Private constructor for singleton
    
    // Texture data for each block type and face
    std::unordered_map<BlockType, std::unordered_map<BlockFace, BlockTexture>> blockData;
    std::unordered_map<BlockType, std::string> blockNames;
    std::unordered_map<BlockType, bool> transparencyData;

    void initializeBlockData();
    void registerBlock(BlockType type, const std::string& name, 
                      const BlockTexture& top, const BlockTexture& bottom,
                      const BlockTexture& sides, bool isTransparent = false);
    void registerBlock(BlockType type, const std::string& name,
                      const BlockTexture& top, const BlockTexture& bottom,
                      const BlockTexture& front, const BlockTexture& back,
                      const BlockTexture& left, const BlockTexture& right,
                      bool isTransparent = false);
};

#endif // BLOCK_DATABASE_H