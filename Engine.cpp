#include "Engine.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_set>
#include <vector>
#include <thread>
#include <chrono>

#include "core/rendering/Renderer.h"

void Engine::init() {
    if (active) {
        return;
    }

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

    Renderer::SetupRendering();

    active = true;
}

void Engine::run() {
    if (!active) {
        init();
    }
    if (!active) return;

    MainLoop();
}

DebugWindow* Engine::OpenDebugWindow(glm::ivec2 position, glm::ivec2 size, bool lightweight) {
    if (!active) {
        init();
    }
    s_debugLightweight = lightweight;
    return DebugWindow::Open(position, size, nullptr, lightweight);
}

DebugWindow* Engine::OpenDebugWindow(bool lightweight) {
    if (!active) {
        init();
    }
    s_debugLightweight = lightweight;
    glm::ivec2 size = lightweight ? glm::ivec2(380, 160) : glm::ivec2(720, 580);
    return DebugWindow::Open(glm::ivec2(40, 40), size, nullptr, lightweight);
}

void Engine::CloseDebugWindow() {
    DebugWindow::CloseAll();
}

DebugWindow* Engine::GetDebugWindow() {
    return DebugWindow::GetInstance();
}

bool Engine::IsDebugWindowOpen() {
    return DebugWindow::IsOpenStatic();
}

void Engine::SetDebugWindowLightweight(bool lightweight) {
    s_debugLightweight = lightweight;
    if (DebugWindow* win = DebugWindow::GetInstance()) {
        win->SetLightweight(lightweight);
    }
}

bool Engine::IsDebugWindowLightweight() {
    if (DebugWindow* win = DebugWindow::GetInstance()) {
        return win->IsLightweight();
    }
    return s_debugLightweight;
}

void Engine::terminate() {
    if (!active) return;
    active = false;

    DebugWindow::CloseAll();

    // Unload all loaded scenes while OpenGL contexts are still valid
    std::vector<Scene*> allScenes = Scene::GetAllScenes();
    for (Scene* scene : allScenes) {
        if (scene && scene->GetState() != SceneState::Unloaded) {
            scene->Unload();
        }
    }

    // Close all open GLFW windows before terminating GLFW
    std::vector<Window*> openWindows = Window::GetAllWindows();
    for (Window* win : openWindows) {
        if (win && win->IsOpen()) {
            win->Close();
        }
    }

    Timer::ClearAll();
    Tween::ClearAll();

    glfwTerminate();
}

void Engine::UpdateScenes() {
    if (!active) return;

    const auto& windows = Window::GetAllWindows();

    // Collect unique target scenes across all open windows so a shared scene
    // is updated only once per frame even if multiple windows view it.
    std::unordered_set<Scene*> scenesToUpdate;

    if (windows.empty()) {
        // Headless mode (no windows created): update all existing scenes
        for (Scene* scene : Scene::GetAllScenes()) {
            if (scene) {
                scenesToUpdate.insert(scene);
            }
        }
    } else {
        for (Window* win : windows) {
            if (win && win->IsOpen()) {
                if (Scene* target = win->GetScene()) {
                    scenesToUpdate.insert(target);
                }
            }
        }
    }

    if (scenesToUpdate.empty()) return;

    // Ensure OpenGL context is current on the primary game window during updates
    if (Window* primary = Window::GetPrimaryWindow()) {
        primary->MakeContextCurrent();
    }

    for (Scene* scene : scenesToUpdate) {
        if (!active) break;
        if (scene->GetState() == SceneState::Unloaded) {
            scene->Load();
            if (!active) break;
        }
        scene->Update();
    }
}

void Engine::RenderWindows() {
    if (!active) return;

    // Snapshot open windows in case callbacks close or open windows mid-frame
    std::vector<Window*> windows = Window::GetAllWindows();
    bool vsyncAppliedThisFrame = false;
    const bool profiling = Profiler::IsEnabled();

    for (Window* win : windows) {
        if (!active) break;
        if (!win || !win->IsOpen()) continue;

        glm::ivec2 fbSize = win->GetFramebufferSize();
        if (fbSize.x <= 0 || fbSize.y <= 0) {
            // Skip rendering minimized or zero-area windows
            continue;
        }

        win->MakeContextCurrent();

        if (win->IsDebugWindow()) {
            if (profiling) Profiler::BeginSection(ProfileID::DebugOverlay);
            win->Render();
            if (profiling) Profiler::EndSection(ProfileID::DebugOverlay);
        } else {
            if (profiling) Profiler::BeginSection(ProfileID::Render3D);
            win->Render();
            if (profiling) Profiler::EndSection(ProfileID::Render3D);
        }

        if (profiling) Profiler::BeginSection(ProfileID::BufferSwap);
        // Pace the frame on the first presented window so N windows don't divide FPS by N
        glfwSwapInterval(vsyncAppliedThisFrame ? 0 : 1);
        vsyncAppliedThisFrame = true;

        win->SwapBuffers();
        if (profiling) Profiler::EndSection(ProfileID::BufferSwap);
    }

    // Ensure OpenGL context is returned to the primary window after all windows render
    if (Window* primary = Window::GetPrimaryWindow()) {
        primary->MakeContextCurrent();
    }

    // Throttle CPU usage when running without windows or when all windows are minimized
    // to prevent crashes and excessive CPU usage.
    if (!vsyncAppliedThisFrame && active) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Engine::MainLoop() {
    double lastFrameTime = glfwGetTime();

    while (active) {
        const bool profiling = Profiler::IsEnabled();

        if (profiling) Profiler::BeginFrame();

        double currentFrameTime = glfwGetTime();
        Time::Update(&lastFrameTime, currentFrameTime);
        deltaTime = static_cast<float>(Time::deltaTime);

        // 1. Events & Input Section
        if (profiling) Profiler::BeginSection(ProfileID::EventsInput);
        glfwPollEvents();

        // Close any individual windows that requested to close
        std::vector<Window*> currentWindows = Window::GetAllWindows();
        for (Window* win : currentWindows) {
            if (!active) break;
            if (win && win->GLWindowShouldClose()) {
                win->Close();
            }
        }
        if (profiling) Profiler::EndSection(ProfileID::EventsInput);

        if (!active) break;

        // Check if any non-debug (game) window is open
        int openGameWindows = 0;
        for (Window* win : Window::GetAllWindows()) {
            if (win && win->IsOpen() && !win->IsDebugWindow()) {
                openGameWindows++;
            }
        }

        // When the other (game) windows are closed, close the debug window as well
        if (openGameWindows == 0) {
            DebugWindow::CloseAll();
            break;
        }

        // 2. Timers & Tweens Section
        if (profiling) Profiler::BeginSection(ProfileID::TimersTweens);
        Timer::UpdateAll();
        Tween::UpdateAll();
        if (profiling) Profiler::EndSection(ProfileID::TimersTweens);

        if (!active) break;

        // 3. Scene & ECS Update Section
        if (profiling) Profiler::BeginSection(ProfileID::SceneECS);
        UpdateScenes();
        if (profiling) Profiler::EndSection(ProfileID::SceneECS);

        if (!active) break;

        // 4. Render Phase (3D Scene Render, Debug Profiler, Buffer Swap)
        RenderWindows();

        if (profiling) Profiler::EndFrame();
    }

    terminate();
}
