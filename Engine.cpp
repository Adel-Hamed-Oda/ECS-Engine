#include "Engine.h"

#include "core/private_imports.h"

void Engine::init() {
    if (initialized) return;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    initialized = true;
    terminated = false;

    if (glfwGetCurrentContext() != nullptr) {
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    }
}

void Engine::run() {
    SetupEngine();
    if (!initialized) return;

    MainLoop();
}

void Engine::terminate() {
    if (terminated && !initialized) return;

    terminated = true;

    if (initialized) {
        // Unload all loaded scenes while OpenGL contexts are still valid
        std::vector<Scene*> allScenes = Scene::GetAllScenes();
        for (Scene* scene : allScenes) {
            if (scene && scene->GetState() != SceneState::Unloaded) {
                scene->_Unload();
            }
        }

        // Close all open GLFW windows before terminating GLFW
        std::vector<Window*> openWindows = Window::GetAllWindows();
        for (Window* win : openWindows) {
            if (win) {
                win->Close();
            }
        }

        glfwTerminate();
        initialized = false;
    }
}

void Engine::SetWindowScene(Window* window, Scene* scene) {
    if (window) {
        window->SetScene(scene);
    }
}

void Engine::SetWindowScene(int windowId, Scene* scene) {
    if (Window* window = Window::Get(windowId)) {
        window->SetScene(scene);
    }
}

void Engine::SetWindowScene(int windowId, const std::string& sceneName) {
    if (Window* window = Window::Get(windowId)) {
        window->SetScene(Scene::Get(sceneName));
    }
}

void Engine::SetupEngine() {
    terminated = false;
    init();
}

void Engine::UpdateScenes() {
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

    // Ensure an OpenGL context is current if Load/Start/Update creates GPU resources
    if (glfwGetCurrentContext() == nullptr) {
        if (Window* primary = Window::GetPrimaryWindow()) {
            primary->MakeContextCurrent();
        }
    }

    for (Scene* scene : scenesToUpdate) {
        if (terminated) break;
        if (scene->GetState() == SceneState::Unloaded) {
            scene->_Load();
        }
        scene->Update();
    }
}

void Engine::RenderWindows() {
    // Snapshot open windows in case callbacks close or open windows mid-frame
    std::vector<Window*> windows = Window::GetAllWindows();
    bool vsyncAppliedThisFrame = false;

    for (Window* win : windows) {
        if (terminated) break;
        if (!win || !win->IsOpen()) continue;

        glm::ivec2 fbSize = win->GetFramebufferSize();
        if (fbSize.x <= 0 || fbSize.y <= 0) {
            // Skip rendering minimized or zero-area windows
            continue;
        }

        // Direct OpenGL calls to this window's canvas
        // not doing this will make all changes happen to the remaining windows, I guess
        // did I say I hate openGL's way of handling things?
        win->MakeContextCurrent();

        if (glad_glViewport) {
            glViewport(0, 0, fbSize.x, fbSize.y);
        }

        if (glad_glClear) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        if (Scene* targetScene = win->GetScene()) {
            if (targetScene->GetState() == SceneState::Unloaded) {
                targetScene->_Load();
            }
            targetScene->Render();
        }

        // Pace the frame on the first presented window so N windows don't divide FPS by N
        // That's what gemini told me lol
        glfwSwapInterval(vsyncAppliedThisFrame ? 0 : 1);
        vsyncAppliedThisFrame = true;

        win->SwapBuffers();
    }

    // Throttle CPU usage when running without windows or when all windows are minimized
    // to prevent crashes and excessive CPU usage.
    if (!vsyncAppliedThisFrame && !terminated) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Engine::MainLoop() {
    double lastFrameTime = glfwGetTime();

    while (!terminated) {
        double currentFrameTime = glfwGetTime();
        deltaTime = static_cast<float>(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // Close any individual windows that requested to close
        std::vector<Window*> currentWindows = Window::GetAllWindows();
        for (Window* win : currentWindows) {
            if (win && win->GLWindowShouldClose()) {
                win->Close();
            }
        }

        // 1. UPDATE PHASE: Update each unique active scene once per frame
        UpdateScenes(deltaTime);
        if (terminated) break;

        // 2. RENDER PHASE: Render each window's target scene with its viewport & camera
        RenderWindows();
    }

    terminate();
}
