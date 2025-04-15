#include "Tree.h"

// Initialize static members
std::random_device Tree::rd;
std::mt19937 Tree::gen(Tree::rd());
std::uniform_int_distribution<> Tree::trunkHeightDist(4, 7);  // Trees 4-7 blocks tall
std::uniform_int_distribution<> Tree::crownSizeDist(3, 5);    // Crown size 3-5 blocks