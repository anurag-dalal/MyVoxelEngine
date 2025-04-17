#include "Player.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Player::Player(ChunkManager* chunkManager, float playerHeight, float playerWidth) 
    : chunkManager(chunkManager),
      position(0.0f, 0.0f, 0.0f),
      yaw(0.0f),
      pitch(0.0f),
      velocity(0.0f, 0.0f, 0.0f),
      gravity(20.0f),
      jumpForce(8.0f),
      onGround(false),
      moveSpeed(5.0f),
      acceleration(30.0f),
      deceleration(15.0f),
      terminalVelocity(-30.0f),
      height(playerHeight),
      width(playerWidth),
      cameraHeightOffset(0.85f),
      front(0.0f, 0.0f, -1.0f),
      right(1.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f)
{
    updateVectors();
}

void Player::setPosition(const glm::vec3& pos) {
    position = pos;
}

glm::vec3 Player::getPosition() const {
    return position;
}

void Player::setRotation(float newYaw, float newPitch) {
    yaw = newYaw;
    // Constrain pitch between -89 and 89 degrees to prevent flipping
    pitch = std::max(-89.0f, std::min(89.0f, newPitch));
    updateVectors();
}

float Player::getYaw() const {
    return yaw;
}

float Player::getPitch() const {
    return pitch;
}

void Player::moveForward(float deltaTime) {
    // Only use the horizontal component for movement (xz-plane)
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    position += horizontalFront * moveSpeed * deltaTime;
    
    // Handle collision
    handleCollisions();
}

void Player::moveBackward(float deltaTime) {
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    position -= horizontalFront * moveSpeed * deltaTime;
    
    // Handle collision
    handleCollisions();
}

void Player::moveLeft(float deltaTime) {
    position -= right * moveSpeed * deltaTime;
    
    // Handle collision
    handleCollisions();
}

void Player::moveRight(float deltaTime) {
    position += right * moveSpeed * deltaTime;
    
    // Handle collision
    handleCollisions();
}

void Player::jump() {
    if (onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }
}

void Player::update(float deltaTime) {
    // Apply gravity
    applyGravity(deltaTime);
    
    // Update position based on velocity
    position += velocity * deltaTime;
    
    // Handle collisions with the world
    handleCollisions();
    
    // Check if player is on ground
    onGround = false;
    
    // Check for ground beneath the player
    glm::vec3 groundCheckPos = position - glm::vec3(0.0f, 0.05f, 0.0f);
    if (checkCollision(groundCheckPos)) {
        onGround = true;
        if (velocity.y < 0) {
            velocity.y = 0;
        }
    }
}

glm::mat4 Player::getViewMatrix() const {
    // Calculate camera position with height offset
    glm::vec3 cameraPos = position + glm::vec3(0.0f, cameraHeightOffset, 0.0f);
    return glm::lookAt(cameraPos, cameraPos + front, up);
}

glm::vec3 Player::getFrontVector() const {
    return front;
}

glm::vec3 Player::getUpVector() const {
    return up;
}

void Player::spawnRandomly(int maxAttempts) {
    glm::vec3 spawnPos;
    if (findSafeSpawnPosition(spawnPos, maxAttempts)) {
        position = spawnPos;
        velocity = glm::vec3(0.0f);
    } else {
        std::cout << "Failed to find a safe spawn position after " << maxAttempts << " attempts." << std::endl;
        // Fallback to a default position high in the air
        position = glm::vec3(0.0f, 50.0f, 0.0f);
    }
}

bool Player::isOnGround() const {
    return onGround;
}

// Private methods

void Player::updateVectors() {
    // Calculate front vector from yaw and pitch
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    // Recalculate right and up vectors
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
}

bool Player::checkCollision(const glm::vec3& pos) const {
    // Check player's bounding box for collisions with voxels
    
    // Simplify by checking a few key points around the player
    float halfWidth = width / 2.0f;
    float feetHeight = pos.y;
    float headHeight = pos.y + height;
    
    // Check feet level corners
    if (chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, feetHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, feetHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, feetHeight, pos.z + halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, feetHeight, pos.z + halfWidth))) {
        return true;
    }
    
    // Check middle level corners
    float middleHeight = pos.y + height / 2.0f;
    if (chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, middleHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, middleHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, middleHeight, pos.z + halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, middleHeight, pos.z + halfWidth))) {
        return true;
    }
    
    // Check head level corners
    if (chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, headHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, headHeight, pos.z - halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x - halfWidth, headHeight, pos.z + halfWidth)) ||
        chunkManager->isVoxelSolid(glm::vec3(pos.x + halfWidth, headHeight, pos.z + halfWidth))) {
        return true;
    }
    
    return false;
}

bool Player::findSafeSpawnPosition(glm::vec3& spawnPos, int maxAttempts) {
    // Random number generation for spawn position
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Define a reasonable spawn area (adjust based on your world size)
    std::uniform_int_distribution<> xDist(-100, 100);
    std::uniform_int_distribution<> zDist(-100, 100);
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // Generate random x,z coordinates
        int x = xDist(gen);
        int z = zDist(gen);
        
        // Find the highest non-air block at this x,z position
        for (int y = 60; y >= 0; y--) {  // Start from a reasonable height
            glm::vec3 checkPos(x, y, z);
            
            if (chunkManager->isVoxelSolid(checkPos)) {
                // Found a solid block, check if there's enough space above it
                glm::vec3 playerPos(x, y + 1, z);  // +1 to be above the ground block
                
                // Check if there's enough space for the player (height blocks of air)
                bool spaceClear = true;
                for (int h = 0; h < ceil(height); h++) {
                    if (chunkManager->isVoxelSolid(glm::vec3(x, y + 1 + h, z))) {
                        spaceClear = false;
                        break;
                    }
                }
                
                if (spaceClear) {
                    spawnPos = playerPos;
                    return true;
                }
                
                // If there's not enough space, no need to check deeper at this x,z
                break;
            }
        }
    }
    
    return false;
}

void Player::applyGravity(float deltaTime) {
    if (!onGround) {
        velocity.y -= gravity * deltaTime;
        
        // Terminal velocity
        velocity.y = std::max(velocity.y, terminalVelocity);
    }
}

void Player::handleCollisions() {
    // Check for collisions at the new position
    if (checkCollision(position)) {
        // Simple resolution: restore previous position
        // In a more advanced implementation, you'd slide along surfaces
        
        // Move slightly higher to prevent getting stuck
        position.y += 0.1f;
        
        // If still colliding, handle more complex resolution
        if (checkCollision(position)) {
            // Try alternative resolutions like moving only along certain axes
            // This is a simplified collision response
            velocity = glm::vec3(0.0f);
        }
    }
}