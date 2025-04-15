#include "HeightMapGenerator.h"
#include <cstdlib> // for rand()
#include <ctime>   // for time()

HeightMapGenerator::HeightMapGenerator(int width, int height, float minHeight, float maxHeight)
    : width(width), height(height), minHeight(minHeight), maxHeight(maxHeight) {
    std::srand(static_cast<unsigned>(std::time(nullptr))); // seed random generator
}

std::vector<std::vector<float>> HeightMapGenerator::generateRandom() {
    std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float normalized = static_cast<float>(rand()) / RAND_MAX;
            heightmap[y][x] = minHeight + normalized * (maxHeight - minHeight);
        }
    }
    return heightmap;
}

int HeightMapGenerator::getWidth() const {
    return width;
}

int HeightMapGenerator::getHeight() const {
    return height;
}
