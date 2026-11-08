#include "Engine.h"

#include "core/private_imports.h"

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

    if (glfwGetCurrentContext() != nullptr) {
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    }
}

void Engine::MainLoop()
{
    float lastFrameTime = 0.0f;

    while (!terminated)
    {
        glfwPollEvents();

        Window* primary = Window::GetPrimaryWindow();
        if (primary != nullptr)
        {
            if (primary->GLWindowShouldClose())
            {
                delete primary;
            }
            else
            {
                Scene* currentScene = Scene::GetCurrentScene();
                if (currentScene != nullptr)
                {
                    if (!currentScene->IsStarted()) {
                        currentScene->Start();
                    } else {
                        currentScene->Update();
                    }

                    glClearColor(
                        0.0f,
                        0.0f,
                        0.0f,
                        1.0f
                    );
                    glClear(
                        GL_COLOR_BUFFER_BIT |
                        GL_DEPTH_BUFFER_BIT
                    );
                    currentScene->Render();
                }

                primary->SwapBuffers();
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    terminate();
}
