#pragma once

#include "core/private_imports.h"

class Mouse {
public:
    static void SetPosition(float x, float y) {
        if (!glfwGetCurrentContext()) return;
        glfwSetCursorPos(glfwGetCurrentContext(), static_cast<double>(x), static_cast<double>(y));
    }

    static void SetCursorVisibility(bool visible) {
        if (!glfwGetCurrentContext()) return;
        glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    static glm::vec2 GetPosition() {
        if (!glfwGetCurrentContext()) return glm::vec2(0.0f);
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        return glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    static glm::vec2 GetDelta() {
        if (!glfwGetCurrentContext()) return glm::vec2(0.0f);
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        static glm::vec2 lastMousePos(static_cast<float>(xpos), static_cast<float>(ypos));
        glm::vec2 mouseDelta(static_cast<float>(xpos) - lastMousePos.x, static_cast<float>(ypos) - lastMousePos.y);
        lastMousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        return mouseDelta;
    }
};