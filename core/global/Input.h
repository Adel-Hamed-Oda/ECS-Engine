#pragma once

#include "core/private_imports.h"
#include "KeyCode.h"

class Input {
public:
    static bool IsKeyPressed(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_PRESS;
    }

    static bool IsKeyReleased(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_RELEASE;
    }

    static bool IsKeyDown(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_REPEAT;
    }
};