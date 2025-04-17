#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../World/ChunkManager.h"
#include <random>

class Player {
public:
    Player(ChunkManager* chunkManager, float playerHeight = 1.8f, float playerWidth = 0.6f);
    ~Player() = default;
    
    // Position and movement
    void setPosition(const glm::vec3& pos);
    glm::vec3 getPosition() const;
    void setRotation(float yaw, float pitch);
    float getYaw() const;
    float getPitch() const;
    
    // Movement methods
    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void jump();
    
    // Physics update
    void update(float deltaTime);
    
    // Camera
    glm::mat4 getViewMatrix() const;
    glm::vec3 getFrontVector() const;
    glm::vec3 getUpVector() const;
    
    // Camera height offset
    void setCameraHeightOffset(float offset) { cameraHeightOffset = offset; }
    float getCameraHeightOffset() const { return cameraHeightOffset; }
    
    // Physics properties setters
    void setGravity(float value) { gravity = value; }
    void setJumpForce(float value) { jumpForce = value; }
    void setMoveSpeed(float value) { moveSpeed = value; }
    void setTerminalVelocity(float value) { terminalVelocity = value; }
    void setAcceleration(float value) { acceleration = value; }
    void setDeceleration(float value) { deceleration = value; }
    void setPlayerDimensions(float height, float width) { this->height = height; this->width = width; }
    
    // Spawn at a random valid location
    void spawnRandomly(int maxAttempts = 100);
    
    // Check if the player is on the ground
    bool isOnGround() const;
    
private:
    // Position and orientation
    glm::vec3 position;
    float yaw;   // Horizontal rotation (around Y-axis) in degrees
    float pitch; // Vertical rotation (around X-axis) in degrees
    
    // Physics properties
    glm::vec3 velocity;
    float gravity;
    float jumpForce;
    bool onGround;
    
    // Movement properties
    float moveSpeed;
    float acceleration;
    float deceleration;
    float terminalVelocity;
    
    // Player dimensions
    float height;
    float width;
    
    // Reference to the chunk manager for collision detection
    ChunkManager* chunkManager;
    
    // Derived vectors (recalculated when rotation changes)
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    
    // Camera height offset (distance above player position)
    float cameraHeightOffset;
    
    // Recalculate the front, right and up vectors based on yaw and pitch
    void updateVectors();
    
    // Check for collision with voxels
    bool checkCollision(const glm::vec3& pos) const;
    
    // Find a suitable spawn position
    bool findSafeSpawnPosition(glm::vec3& spawnPos, int maxAttempts);
    
    // Physics helper methods
    void applyGravity(float deltaTime);
    void handleCollisions();
};

#endif // PLAYER_H