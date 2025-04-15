#pragma once

#include <vector>

class HeightMapGenerator {
public:
    HeightMapGenerator(int width, int height, float minHeight = 0.0f, float maxHeight = 10.0f);

    std::vector<std::vector<float>> generateRandom();
    
    // Future: std::vector<std::vector<float>> generatePerlin(float frequency, int octaves);

    int getWidth() const;
    int getHeight() const;

private:
    int width;
    int height;
    float minHeight;
    float maxHeight;
};
