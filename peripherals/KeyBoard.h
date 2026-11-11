#pragma once

#include "core/private_imports.h"
#include "KeyCode.h"

class KeyBoard {
public:
    static bool IsPressed(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_PRESS;
    }

    static bool IsReleased(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_RELEASE;
    }

    static bool IsDown(KeyCode key) {
        if (!glfwGetCurrentContext()) return false;
        return glfwGetKey(glfwGetCurrentContext(), static_cast<int>(key)) == GLFW_REPEAT;
    }
};