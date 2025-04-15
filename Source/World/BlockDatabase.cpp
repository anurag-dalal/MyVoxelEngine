#include "BlockDatabase.h"

BlockDatabase& BlockDatabase::getInstance() {
    static BlockDatabase instance;
    return instance;
}

BlockDatabase::BlockDatabase() {
    initializeBlockData();
}

void BlockDatabase::initializeBlockData() {
    // Register all block types with their textures
    
    // AIR block (transparent)
    registerBlock(BlockType::AIR, "Air",
                 BlockTexture(glm::vec2(0, 0), true),  // All faces transparent
                 BlockTexture(glm::vec2(0, 0), true),
                 BlockTexture(glm::vec2(0, 0), true),
                 true);

    // GRASS block
    registerBlock(BlockType::GRASS, "Grass",
                 BlockTexture(glm::vec2(0, 0)),  // Top (green grass)
                 BlockTexture(glm::vec2(2, 0)),  // Bottom (dirt)
                 BlockTexture(glm::vec2(3, 0))); // Sides (grass side)

    // DIRT block
    registerBlock(BlockType::DIRT, "Dirt",
                 BlockTexture(glm::vec2(2, 0)),  // All faces dirt
                 BlockTexture(glm::vec2(2, 0)),
                 BlockTexture(glm::vec2(2, 0)));

    // STONE block
    registerBlock(BlockType::STONE, "Stone",
                 BlockTexture(glm::vec2(1, 0)),  // All faces stone
                 BlockTexture(glm::vec2(1, 0)),
                 BlockTexture(glm::vec2(1, 0)));

    // WOOD_LOG block
    registerBlock(BlockType::WOOD_LOG, "Wood Log",
                 BlockTexture(glm::vec2(5, 0)),  // Top (wood end)
                 BlockTexture(glm::vec2(5, 0)),  // Bottom (wood end)
                 BlockTexture(glm::vec2(4, 0))); // Sides (wood bark)

    // LEAVES block (slightly transparent)
    registerBlock(BlockType::LEAVES, "Leaves",
                 BlockTexture(glm::vec2(6, 0), true),  // All faces leaves
                 BlockTexture(glm::vec2(6, 0), true),
                 BlockTexture(glm::vec2(6, 0), true),
                 true);

    // SAND block
    registerBlock(BlockType::SAND, "Sand",
                 BlockTexture(glm::vec2(7, 0)),  // All faces sand
                 BlockTexture(glm::vec2(7, 0)),
                 BlockTexture(glm::vec2(7, 0)));

    // WATER block (transparent)
    registerBlock(BlockType::WATER, "Water",
                 BlockTexture(glm::vec2(8, 0), true),  // All faces water
                 BlockTexture(glm::vec2(8, 0), true),
                 BlockTexture(glm::vec2(8, 0), true),
                 true);
}

void BlockDatabase::registerBlock(BlockType type, const std::string& name,
                                const BlockTexture& top, const BlockTexture& bottom,
                                const BlockTexture& sides, bool isTransparent) {
    blockNames[type] = name;
    transparencyData[type] = isTransparent;
    
    blockData[type][BlockFace::TOP] = top;
    blockData[type][BlockFace::BOTTOM] = bottom;
    blockData[type][BlockFace::FRONT] = sides;
    blockData[type][BlockFace::BACK] = sides;
    blockData[type][BlockFace::LEFT] = sides;
    blockData[type][BlockFace::RIGHT] = sides;
}

void BlockDatabase::registerBlock(BlockType type, const std::string& name,
                                const BlockTexture& top, const BlockTexture& bottom,
                                const BlockTexture& front, const BlockTexture& back,
                                const BlockTexture& left, const BlockTexture& right,
                                bool isTransparent) {
    blockNames[type] = name;
    transparencyData[type] = isTransparent;
    
    blockData[type][BlockFace::TOP] = top;
    blockData[type][BlockFace::BOTTOM] = bottom;
    blockData[type][BlockFace::FRONT] = front;
    blockData[type][BlockFace::BACK] = back;
    blockData[type][BlockFace::LEFT] = left;
    blockData[type][BlockFace::RIGHT] = right;
}

BlockTexture BlockDatabase::getBlockTexture(BlockType type, BlockFace face) const {
    if (blockData.find(type) == blockData.end()) {
        return BlockTexture(); // Return default texture if block type not found
    }
    
    const auto& faceMap = blockData.at(type);
    if (faceMap.find(face) == faceMap.end()) {
        return BlockTexture(); // Return default texture if face not found
    }
    
    return faceMap.at(face);
}

bool BlockDatabase::isBlockTransparent(BlockType type) const {
    auto it = transparencyData.find(type);
    return it != transparencyData.end() && it->second;
}

std::string BlockDatabase::getBlockName(BlockType type) const {
    auto it = blockNames.find(type);
    return it != blockNames.end() ? it->second : "Unknown";
}