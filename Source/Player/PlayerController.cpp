#include "PlayerController.h"
#include <iostream>

PlayerController::PlayerController(Player* player, GLFWwindow* window)
    : player(player),
      window(window),
      moveForward(false),
      moveBackward(false),
      moveLeft(false),
      moveRight(false),
      sprint(false),
      jumping(false),
      lastX(0.0),
      lastY(0.0),
      firstMouse(true)
{
}

void PlayerController::init(const PlayerConfig& config) {
    this->config = config;
    
    // Apply physics settings to player
    applyPlayerPhysics(config.physics);
    
    // Get window size to center mouse
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = width / 2.0;
    lastY = height / 2.0;
}

void PlayerController::applyPlayerPhysics(const PlayerPhysicsConfig& physics) {
    if (player) {
        player->setCameraHeightOffset(physics.cameraHeightOffset);
        player->setPlayerDimensions(physics.height, physics.width);
        player->setGravity(physics.gravity);
        player->setJumpForce(physics.jumpForce);
        player->setMoveSpeed(physics.moveSpeed);
        player->setAcceleration(physics.acceleration);
        player->setDeceleration(physics.deceleration);
        player->setTerminalVelocity(physics.terminalVelocity);
    }
}

void PlayerController::processKeyboardInput(float deltaTime) {
    if (!player || !window) return;
    
    // Update movement state based on key presses
    moveForward = glfwGetKey(window, config.controls.keyboard.moveForward) == GLFW_PRESS;
    moveBackward = glfwGetKey(window, config.controls.keyboard.moveBackward) == GLFW_PRESS;
    moveLeft = glfwGetKey(window, config.controls.keyboard.moveLeft) == GLFW_PRESS;
    moveRight = glfwGetKey(window, config.controls.keyboard.moveRight) == GLFW_PRESS;
    sprint = glfwGetKey(window, config.controls.keyboard.sprint) == GLFW_PRESS;
    
    // Apply movement based on state
    float speedMultiplier = sprint ? 1.5f : 1.0f;
    
    if (moveForward)
        player->moveForward(deltaTime * speedMultiplier);
    if (moveBackward)
        player->moveBackward(deltaTime * speedMultiplier);
    if (moveLeft)
        player->moveLeft(deltaTime * speedMultiplier);
    if (moveRight)
        player->moveRight(deltaTime * speedMultiplier);
    
    // Handle jumping
    if (glfwGetKey(window, config.controls.keyboard.jump) == GLFW_PRESS && !jumping) {
        player->jump();
        jumping = true;
    }
    
    if (glfwGetKey(window, config.controls.keyboard.jump) == GLFW_RELEASE) {
        jumping = false;
    }
    
    // Handle respawning
    if (glfwGetKey(window, config.controls.keyboard.respawn) == GLFW_PRESS) {
        player->spawnRandomly();
    }
    
    // Update player physics
    player->update(deltaTime);
}

void PlayerController::processMouseMovement(double xpos, double ypos) {
    if (!player) return;
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    double xoffset = xpos - lastX;
    double yoffset = config.controls.mouse.invertY ? ypos - lastY : lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    float sensitivity = config.controls.mouse.sensitivity;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    // Update player's rotation
    player->setRotation(
        player->getYaw() + xoffset,
        player->getPitch() + yoffset
    );
}

void PlayerController::resetMousePosition(double xpos, double ypos) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = true;
}