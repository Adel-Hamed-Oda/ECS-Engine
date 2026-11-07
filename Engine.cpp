#include "Engine.h"

#include "core/import_files/public_imports.h"
#include "core/import_files/private_imports.h"

#include "core/window/Window.h"

void Engine::run() {
    SetupExternals();
};

void Engine::terminate() {
    if (terminated) return;

    terminated = true;
    glfwTerminate();
};

void SetupExternals() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    gladLoadGL((GLADloadfunc)glfwGetProcAddress);

    new Window("ECS Test Game", glm::vec2(0, 0), glm::vec2(800, 600));
}