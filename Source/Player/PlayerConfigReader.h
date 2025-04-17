#ifndef PLAYER_CONFIG_READER_H
#define PLAYER_CONFIG_READER_H

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct PlayerPhysicsConfig {
    float height;
    float width;
    float cameraHeightOffset;
    float gravity;
    float jumpForce;
    float moveSpeed;
    float acceleration;
    float deceleration;
    float terminalVelocity;
};

struct PlayerControlConfig {
    struct {
        int moveForward;
        int moveBackward;
        int moveLeft;
        int moveRight;
        int jump;
        int sprint;
        int respawn;
    } keyboard;
    
    struct {
        float sensitivity;
        bool invertY;
        float maxPitch;
    } mouse;
};

struct PlayerConfig {
    PlayerPhysicsConfig physics;
    PlayerControlConfig controls;
};

// Function to load player configuration
PlayerConfig loadPlayerConfig(const std::string& configPath);

// Helper function to convert key string to GLFW key code
int keyStringToKeyCode(const std::string& keyString);

#endif // PLAYER_CONFIG_READER_H