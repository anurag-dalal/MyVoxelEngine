#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

// Include GLAD before GLFW to avoid conflicting OpenGL headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include "PlayerConfigReader.h"

class PlayerController {
public:
    PlayerController(Player* player, GLFWwindow* window);
    ~PlayerController() = default;

    // Initialize the controller with configuration
    void init(const PlayerConfig& config);
    
    // Process keyboard input
    void processKeyboardInput(float deltaTime);
    
    // Process mouse movement
    void processMouseMovement(double xpos, double ypos);
    
    // This should be called when mouse first enters the window
    void setFirstMouse(bool first) { firstMouse = first; }
    
    // Reset mouse position (used when entering/exiting menus)
    void resetMousePosition(double xpos, double ypos);

private:
    Player* player;
    GLFWwindow* window;
    PlayerConfig config;
    
    // Movement state
    bool moveForward;
    bool moveBackward;
    bool moveLeft;
    bool moveRight;
    bool sprint;
    bool jumping;
    
    // Mouse position tracking
    double lastX;
    double lastY;
    bool firstMouse;
    
    // Apply control maps from config
    void remapControls(const PlayerConfig& config);
    
    // Applies physics settings to player
    void applyPlayerPhysics(const PlayerPhysicsConfig& physicsConfig);
};

#endif // PLAYER_CONTROLLER_H