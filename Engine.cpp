#include "Engine.h"

#include "core/private_imports.h"

void Engine::run() {
    SetupExternals();
};

void Engine::terminate() {
    if (terminated) return;

    terminated = true;
    glfwTerminate();
};

void SetupExternals() {
    if (!glfwInit()) { glfwTerminate(); }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    gladLoadGL((GLADloadfunc)glfwGetProcAddress);
}

    /*PublicDomain::CurrentWindow = new Window(CONFIG::WINDOW::WIDTH, CONFIG::WINDOW::HEIGHT, CONFIG::WINDOW::TITLE, CONFIG::WINDOW::FULLSCREEN);
    if (PublicDomain::CurrentWindow == nullptr)
    {
        glfwTerminate();
    }
    glfwMakeContextCurrent(PublicDomain::CurrentWindow->GetGLFWWindow());*/