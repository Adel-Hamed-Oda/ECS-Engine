#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
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