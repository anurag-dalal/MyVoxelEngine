#include "PlayerConfigReader.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Map string key names to GLFW key codes
int keyStringToKeyCode(const std::string& keyString) {
    static std::map<std::string, int> keyMap = {
        {"A", GLFW_KEY_A},
        {"B", GLFW_KEY_B},
        {"C", GLFW_KEY_C},
        {"D", GLFW_KEY_D},
        {"E", GLFW_KEY_E},
        {"F", GLFW_KEY_F},
        {"G", GLFW_KEY_G},
        {"H", GLFW_KEY_H},
        {"I", GLFW_KEY_I},
        {"J", GLFW_KEY_J},
        {"K", GLFW_KEY_K},
        {"L", GLFW_KEY_L},
        {"M", GLFW_KEY_M},
        {"N", GLFW_KEY_N},
        {"O", GLFW_KEY_O},
        {"P", GLFW_KEY_P},
        {"Q", GLFW_KEY_Q},
        {"R", GLFW_KEY_R},
        {"S", GLFW_KEY_S},
        {"T", GLFW_KEY_T},
        {"U", GLFW_KEY_U},
        {"V", GLFW_KEY_V},
        {"W", GLFW_KEY_W},
        {"X", GLFW_KEY_X},
        {"Y", GLFW_KEY_Y},
        {"Z", GLFW_KEY_Z},
        {"SPACE", GLFW_KEY_SPACE},
        {"ESCAPE", GLFW_KEY_ESCAPE},
        {"ENTER", GLFW_KEY_ENTER},
        {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT},
        {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
        {"LEFT_CTRL", GLFW_KEY_LEFT_CONTROL},
        {"RIGHT_CTRL", GLFW_KEY_RIGHT_CONTROL},
        {"LEFT_ALT", GLFW_KEY_LEFT_ALT},
        {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT},
        {"TAB", GLFW_KEY_TAB},
        {"0", GLFW_KEY_0},
        {"1", GLFW_KEY_1},
        {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3},
        {"4", GLFW_KEY_4},
        {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6},
        {"7", GLFW_KEY_7},
        {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9}
    };

    auto it = keyMap.find(keyString);
    if (it != keyMap.end()) {
        return it->second;
    }
    
    std::cerr << "Warning: Unknown key '" << keyString << "', defaulting to 'W'" << std::endl;
    return GLFW_KEY_W; // Default to W if key not found
}

PlayerConfig loadPlayerConfig(const std::string& configPath) {
    PlayerConfig config;
    
    // Set default values
    config.physics.height = 1.8f;
    config.physics.width = 0.6f;
    config.physics.cameraHeightOffset = 0.85f;
    config.physics.gravity = 20.0f;
    config.physics.jumpForce = 8.0f;
    config.physics.moveSpeed = 5.0f;
    config.physics.acceleration = 30.0f;
    config.physics.deceleration = 15.0f;
    config.physics.terminalVelocity = -30.0f;
    
    config.controls.keyboard.moveForward = GLFW_KEY_W;
    config.controls.keyboard.moveBackward = GLFW_KEY_S;
    config.controls.keyboard.moveLeft = GLFW_KEY_A;
    config.controls.keyboard.moveRight = GLFW_KEY_D;
    config.controls.keyboard.jump = GLFW_KEY_SPACE;
    config.controls.keyboard.sprint = GLFW_KEY_LEFT_SHIFT;
    config.controls.keyboard.respawn = GLFW_KEY_R;
    
    config.controls.mouse.sensitivity = 0.1f;
    config.controls.mouse.invertY = false;
    config.controls.mouse.maxPitch = 89.0f;
    
    try {
        // Read the JSON file
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open player config file: " << configPath << std::endl;
            std::cerr << "Using default player settings." << std::endl;
            return config;
        }
        
        json j;
        file >> j;
        
        // Parse physics settings
        if (j.contains("physics")) {
            auto& physics = j["physics"];
            
            if (physics.contains("height")) 
                config.physics.height = physics["height"].get<float>();
                
            if (physics.contains("width")) 
                config.physics.width = physics["width"].get<float>();
                
            if (physics.contains("cameraHeightOffset")) 
                config.physics.cameraHeightOffset = physics["cameraHeightOffset"].get<float>();
                
            if (physics.contains("gravity")) 
                config.physics.gravity = physics["gravity"].get<float>();
                
            if (physics.contains("jumpForce")) 
                config.physics.jumpForce = physics["jumpForce"].get<float>();
                
            if (physics.contains("moveSpeed")) 
                config.physics.moveSpeed = physics["moveSpeed"].get<float>();
                
            if (physics.contains("acceleration")) 
                config.physics.acceleration = physics["acceleration"].get<float>();
                
            if (physics.contains("deceleration")) 
                config.physics.deceleration = physics["deceleration"].get<float>();
                
            if (physics.contains("terminalVelocity")) 
                config.physics.terminalVelocity = physics["terminalVelocity"].get<float>();
        }
        
        // Parse control settings
        if (j.contains("controls")) {
            auto& controls = j["controls"];
            
            if (controls.contains("keyboard")) {
                auto& keyboard = controls["keyboard"];
                
                if (keyboard.contains("moveForward"))
                    config.controls.keyboard.moveForward = keyStringToKeyCode(keyboard["moveForward"].get<std::string>());
                    
                if (keyboard.contains("moveBackward"))
                    config.controls.keyboard.moveBackward = keyStringToKeyCode(keyboard["moveBackward"].get<std::string>());
                    
                if (keyboard.contains("moveLeft"))
                    config.controls.keyboard.moveLeft = keyStringToKeyCode(keyboard["moveLeft"].get<std::string>());
                    
                if (keyboard.contains("moveRight"))
                    config.controls.keyboard.moveRight = keyStringToKeyCode(keyboard["moveRight"].get<std::string>());
                    
                if (keyboard.contains("jump"))
                    config.controls.keyboard.jump = keyStringToKeyCode(keyboard["jump"].get<std::string>());
                    
                if (keyboard.contains("sprint"))
                    config.controls.keyboard.sprint = keyStringToKeyCode(keyboard["sprint"].get<std::string>());
                    
                if (keyboard.contains("respawn"))
                    config.controls.keyboard.respawn = keyStringToKeyCode(keyboard["respawn"].get<std::string>());
            }
            
            if (controls.contains("mouse")) {
                auto& mouse = controls["mouse"];
                
                if (mouse.contains("sensitivity"))
                    config.controls.mouse.sensitivity = mouse["sensitivity"].get<float>();
                    
                if (mouse.contains("invertY"))
                    config.controls.mouse.invertY = mouse["invertY"].get<bool>();
                    
                if (mouse.contains("maxPitch"))
                    config.controls.mouse.maxPitch = mouse["maxPitch"].get<float>();
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing player config: " << e.what() << std::endl;
        std::cerr << "Using default player settings." << std::endl;
    }
    
    return config;
}