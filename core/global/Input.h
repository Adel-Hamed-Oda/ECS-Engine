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

    static glm::vec2 GetMousePosition() {
        if (!glfwGetCurrentContext()) return glm::vec2(0.0f);
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        return glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    static void SetMousePosition(float x, float y) {
        if (!glfwGetCurrentContext()) return;
        glfwSetCursorPos(glfwGetCurrentContext(), static_cast<double>(x), static_cast<double>(y));
    }

    static glm::vec2 GetMouseDelta() {
        if (!glfwGetCurrentContext()) return glm::vec2(0.0f);
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        static glm::vec2 lastMousePos(static_cast<float>(xpos), static_cast<float>(ypos));
        glm::vec2 mouseDelta(static_cast<float>(xpos) - lastMousePos.x, static_cast<float>(ypos) - lastMousePos.y);
        lastMousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        return mouseDelta;
    }
};