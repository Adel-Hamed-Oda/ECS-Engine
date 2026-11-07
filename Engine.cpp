#include "Engine.h"

#include "core/import_files/private_imports.h"

void Engine::run() {
    SetupEngine();

    MainLoop();
};

void Engine::terminate() {
    if (terminated) return;

    terminated = true;
    glfwTerminate();
};

void Engine::SetupEngine() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (Window::GetPrimaryWindow() == nullptr) {
        new Window("ECS Test Game", glm::vec2(100, 100), glm::vec2(800, 600));
    }

    gladLoadGL((GLADloadfunc)glfwGetProcAddress);
}

void Engine::MainLoop()
{
    float lastFrameTime = 0.0f;

    while (!terminated)
    {
        Window* primary = Window::GetPrimaryWindow();
        if (!primary || primary->GLWindowShouldClose()) {
            break;
        }

        glfwPollEvents();

        glClearColor(
            0,
            0,
            0,
            0
        );

        // clears the background color and depth buffer
        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );
            
        primary->SwapBuffers();
    }

    terminate();
}
